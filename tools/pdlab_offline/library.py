"""Offline-only curated Pd library. Standard library, no Pd execution/import hooks."""
import argparse
import hashlib
import json
import math
import os
from pathlib import Path
import re
import shutil
import stat
import tempfile

BASE = Path(__file__).resolve().parent
PROFILE = 'vanilla-fx-current-1'
ALLOWED = {'inlet~', 'outlet~', 'r', 'pack', 'line~', '*~', '+~', '-~', 'sig~', 'lop~'}

class Invalid(ValueError):
    pass

def require(ok, message):
    if not ok:
        raise Invalid(message)

def pairs(items):
    out = {}
    for key, value in items:
        require(key not in out, 'duplicate JSON key: ' + key)
        out[key] = value
    return out

def loads(data):
    try:
        return json.loads(data.decode('utf-8'), object_pairs_hook=pairs,
                          parse_constant=lambda x: (_ for _ in ()).throw(Invalid('nonfinite JSON')))
    except (UnicodeError, ValueError, RecursionError) as exc:
        raise Invalid('invalid JSON: ' + str(exc)) from exc

def canonical(obj):
    return json.dumps(obj, sort_keys=True, separators=(',', ':'), ensure_ascii=True,
                      allow_nan=False).encode('ascii') + b'\n'

def sha(data):
    return hashlib.sha256(data).hexdigest()

def relative(name):
    require(isinstance(name, str) and len(name) <= 240 and
            all(re.fullmatch(r'[A-Za-z0-9_-][A-Za-z0-9_.-]*', s)
                for s in name.split('/')), 'unsafe relative path')
    return name

def regular_tree(root):
    """Bounded no-symlink snapshot; reject special files, hardlinks and executable files.

    Caller must own a quiescent offline tree. This is not adversarial FS isolation.
    """
    root = Path(root).absolute()
    for p in [root, *root.parents]:
        require(stat.S_ISDIR(p.lstat().st_mode), 'symlink/non-directory ancestor')
    files = {}
    total = 0
    directory_count = 0
    for current, dirs, names in os.walk(root, followlinks=False):
        for name in dirs + names:
            p = Path(current) / name
            rel = relative(p.relative_to(root).as_posix())
            st = p.lstat()
            require(not stat.S_ISLNK(st.st_mode), 'symlink rejected')
            require(len(Path(rel).parts) <= 4, 'depth limit')
            if name in dirs:
                directory_count += 1
                require(directory_count <= 256, 'directory quota')
                require(stat.S_ISDIR(st.st_mode), 'invalid directory')
                continue
            require(stat.S_ISREG(st.st_mode) and st.st_nlink == 1 and not st.st_mode & 0o111,
                    'special/hardlinked/executable file rejected')
            require(st.st_size <= 131072 and len(files) < 64, 'file quota')
            fd = os.open(p, os.O_RDONLY | getattr(os, 'O_NOFOLLOW', 0) | os.O_NONBLOCK)
            try:
                before = os.fstat(fd)
                require((before.st_dev, before.st_ino) == (st.st_dev, st.st_ino), 'file changed')
                with os.fdopen(fd, 'rb', closefd=False) as stream:
                    data = stream.read(131073)
                after = os.fstat(fd)
                require(stat.S_ISREG(after.st_mode) and (before.st_dev, before.st_ino, before.st_size, before.st_mtime_ns, before.st_ctime_ns, before.st_mode, before.st_nlink) ==
                        (after.st_dev, after.st_ino, after.st_size, after.st_mtime_ns, after.st_ctime_ns, after.st_mode, after.st_nlink) and len(data) == st.st_size,
                        'file changed while reading')
            finally:
                os.close(fd)
            total += len(data)
            require(total <= 1048576, 'package quota')
            files[rel] = data
    return files

def schema_check(value, schema, where='$'):
    """Validator for the closed JSON Schema subset used by manifest.schema.json.

    Not a general JSON Schema implementation. Supported keywords audited in tests.
    """
    types = {'object': dict, 'array': list, 'string': str, 'integer': int,
             'number': (int, float), 'boolean': bool}
    kind = schema.get('type')
    if kind:
        require(isinstance(value, types[kind]) and
                not (kind in ('integer', 'number') and isinstance(value, bool)), where + ': type')
    if isinstance(value, (int, float)):
        require(math.isfinite(value), where + ': finite')
    if 'const' in schema:
        require(type(value) is type(schema['const']) and value == schema['const'], where + ': const')
    if 'enum' in schema:
        require(value in schema['enum'], where + ': enum')
    if kind == 'object':
        props = schema['properties']
        require(set(schema.get('required', [])) <= value.keys(), where + ': missing fields')
        require(schema.get('additionalProperties') is False and value.keys() <= props.keys(), where + ': extra fields')
        for k, v in value.items():
            schema_check(v, props[k], where + '.' + k)
    if kind == 'array':
        require(schema.get('minItems', 0) <= len(value) <= schema.get('maxItems', 1000), where + ': length')
        if schema.get('uniqueItems'):
            require(len({canonical(x) for x in value}) == len(value), where + ': duplicate items')
        for v in value:
            schema_check(v, schema['items'], where + '[]')
    if kind == 'string':
        require(schema.get('minLength', 0) <= len(value) <= schema.get('maxLength', 1000), where + ': length')
        require(not any(ord(c) < 32 for c in value), where + ': control text')
        if 'pattern' in schema:
            require(re.fullmatch(schema['pattern'], value) is not None, where + ': pattern')
    if kind in ('number', 'integer'):
        require(schema.get('minimum', -1e300) <= value <= schema.get('maximum', 1e300), where + ': range')

def audit_pd(data, abstractions):
    try:
        text = data.decode('ascii')
    except UnicodeError as exc:
        raise Invalid('Pd source must be ASCII in current profile') from exc
    require(text.strip() and '\x00' not in text and '\\' not in text, 'unsupported Pd encoding/escape')
    statements = text.split(';')
    require(not statements[-1].strip(), 'truncated Pd source')
    objects, deps, connections = [], set(), []
    canvas_seen = False
    for statement in statements[:-1]:
        t = statement.split()
        require(t, 'empty Pd statement')
        if t[:2] == ['#N', 'canvas']:
            require(not canvas_seen and not objects and not connections and len(t) == 7, 'nested/invalid canvas')
            canvas_seen = True
        elif t[:2] == ['#X', 'obj']:
            require(len(t) >= 5, 'invalid object')
            require(canvas_seen, 'object before canvas')
            obj, args = t[4], t[5:]
            require(obj in ALLOWED or obj in abstractions, 'unsupported object: ' + obj)
            require(obj != 'r' or args in [['ul-gain'], ['ul-mix']], 'unsupported receiver')
            if obj in abstractions:
                require(not args, 'abstraction arguments unsupported')
                deps.add(obj + '.pd')
            objects.append(obj)
        elif t[:2] == ['#X', 'connect']:
            require(len(t) == 6 and all(x.isdigit() for x in t[2:]), 'invalid connection')
            connections.append([int(x) for x in t[2:]])
        elif t[:2] != ['#X', 'text']:
            raise Invalid('unsupported Pd statement')
    for src, outlet, dst, inlet in connections:
        require(src < len(objects) and dst < len(objects), 'connection object out of range')
    require(objects and canvas_seen, 'empty graph')
    return set(objects) & ALLOWED, deps

def validate_patch(root):
    files = regular_tree(root)
    require('manifest.json' in files and len(files['manifest.json']) <= 16384, 'manifest missing/too large')
    m = loads(files['manifest.json'])
    schema_check(m, loads((BASE / 'manifest.schema.json').read_bytes()))
    require(m['runtime']['profile'] == PROFILE, 'unsupported profile')
    require(m['runtime']['entry'] == 'main.pd', 'entry must be main.pd')
    require(not m['dependencies']['externals'], 'externals rejected')
    require(not m['assets'], 'assets not supported by current profile')
    require(m['source']['sha256'] == sha(files.get('main.pd', b'')), 'source hash mismatch')
    require(all(p.endswith('.pd') or p in {'manifest.json', 'LICENSE.txt', 'PROVENANCE.json'} for p in files),
            'unsupported runtime/metadata file')
    require('PROVENANCE.json' in files, 'provenance missing')
    provenance = loads(files['PROVENANCE.json'])
    require(isinstance(provenance, dict) and provenance.get('sourceCommit') == m['source']['revision'] and
            provenance.get('sourcePath') == m['source']['path'] and
            provenance.get('sourceSha256') == m['source']['sha256'], 'provenance mismatch')
    records = m['files']
    paths = [relative(r['path']) for r in records]
    require(len(paths) == len(set(paths)), 'duplicate file')
    require(set(paths) == files.keys() - {'manifest.json'}, 'closed file inventory mismatch')
    for record in records:
        data = files[record['path']]
        require(sha(data) == record['sha256'] and len(data) == record['bytes'], 'content hash/size mismatch')
    licenses = {lic['id']: lic for lic in m['licenses']}
    require(len(licenses) == len(m['licenses']), 'duplicate license')
    for lic in licenses.values():
        require(relative(lic['file']) in files and files[lic['file']].strip(), 'license missing/empty')
    require(all(r['licenseRef'] in licenses for r in records), 'unresolved license reference')
    pds = {p for p in files if p.endswith('.pd')}
    require('main.pd' in pds and all('/' not in p and p != 'host.pd' for p in pds), 'flat Pd profile only')
    require(set(m['dependencies']['abstractions']) == pds - {'main.pd'}, 'abstraction inventory mismatch')
    names = {p[:-3] for p in pds}
    require(not names & ALLOWED, 'abstraction shadows built-in')
    used, graph = set(), {}
    for p in pds:
        objects, edges = audit_pd(files[p], names)
        used |= objects
        graph[p] = edges
    require(used == set(m['dependencies']['vanillaObjects']), 'object dependency inventory mismatch')
    visited, visiting = set(), set()
    def visit(p):
        require(p not in visiting, 'recursive abstraction')
        if p in visited:
            return
        visiting.add(p)
        for q in graph[p]:
            visit(q)
        visiting.remove(p)
        visited.add(p)
    visit('main.pd')
    require(visited == pds, 'unused abstraction')
    controls = m['parameters']
    require(len(controls) == 2, 'current profile requires gain/mix only')
    for i, key in enumerate(('gain', 'mix')):
        match = [c for c in controls if c['id'] == key]
        require(len(match) == 1, 'control identity')
        c = match[0]
        require(c['slot'] == i and c['receiver'] == 'ul-' + key and c['min'] == 0 and c['max'] == 1 and
                c['default'] == 1 and c['curve'] == 'linear' and c['type'] == 'continuous' and
                c['smoothingMs'] == 0, 'current profile control contract')
    require(m['pages'] == [{'id': 'main', 'title': 'Main', 'controlIDs': ['gain', 'mix']}], 'current profile pages')
    # Canonical record array, not raw concatenation. Manifest hashes exact bytes.
    tree = [{'path': p, 'sha256': sha(data), 'bytes': len(data)} for p, data in sorted(files.items())]
    return {'id': m['id'], 'revision': m['revision'], 'manifestSha256': sha(files['manifest.json']),
            'treeSha256': sha(canonical(tree)), 'profile': PROFILE}, files

def approval_records(path):
    doc = loads(Path(path).read_bytes())
    require(isinstance(doc, dict) and set(doc) == {'schemaVersion', 'approvals'} and type(doc['schemaVersion']) is int and doc['schemaVersion'] == 1,
            'approval format')
    out = {}
    for a in doc['approvals']:
        require(set(a) == {'id', 'revision', 'treeSha256', 'reviewer', 'scope', 'licenseDecision'}, 'approval fields')
        require(a['scope'] == 'offline-fixture' and a['licenseDecision'] == 'inherited-notice-local-only' and
                isinstance(a['reviewer'], str) and 0 < len(a['reviewer'].strip()) <= 128, 'approval scope/license decision')
        key = (a['id'], a['revision'])
        require(key not in out, 'duplicate approval')
        out[key] = a
    return out

def build_catalog(root, approvals, previous=None):
    root = Path(root)
    inventory = regular_tree(root)
    manifests = sorted(p for p in inventory if p.endswith('/manifest.json'))
    approved = approval_records(approvals)
    entries = []
    keys = set()
    covered = set()
    for p in manifests:
        folder = p.removesuffix('/manifest.json')
        record, files = validate_patch(root / folder)
        key = (record['id'], record['revision'])
        require(key not in keys, 'duplicate id/revision')
        keys.add(key)
        require(folder == 'patches/' + record['id'] + '/' + record['revision'], 'identity/folder mismatch')
        require(key in approved and approved[key]['treeSha256'] == record['treeSha256'], 'not approved/hash mismatch')
        covered |= {folder + '/' + f for f in files}
        entries.append(dict(record, folder=folder))
    require(covered == inventory.keys(), 'unmanaged library file')
    require(keys == approved.keys(), 'approval inventory mismatch')
    require(entries, 'empty catalog')
    result = {'schemaVersion': 1, 'hashAlgorithm': 'sha256-record-array-v1',
              'scope': 'offline-fixture', 'redistributionApproved': False,
              'patches': sorted(entries, key=lambda x: (x['id'], x['revision']))}
    if previous:
        old = loads(Path(previous).read_bytes())
        # Previous catalog is a trusted baseline supplied by reviewer, not self-authenticating.
        prior = {(x['id'], x['revision']): x['treeSha256'] for x in old['patches']}
        for x in entries:
            key = (x['id'], x['revision'])
            require(key not in prior or prior[key] == x['treeSha256'], 'immutable revision changed')
        require(prior.keys() <= keys, 'previous revisions must remain available')
    return result

def write_new(path, data):
    with open(path, 'xb') as f:
        f.write(data)

def import_patch(source, root, approvals):
    record, files = validate_patch(source)
    approved = approval_records(approvals)
    key = (record['id'], record['revision'])
    require(key in approved and approved[key]['treeSha256'] == record['treeSha256'], 'import not approved')
    root = Path(root).absolute()
    require(root.is_dir(), 'create library root explicitly first')
    regular_tree(root)
    dest = root / 'patches' / record['id'] / record['revision']
    require(not dest.exists(), 'revision exists; never overwrite')
    dest.parent.mkdir(parents=True, exist_ok=True)
    temp = Path(tempfile.mkdtemp(prefix='import-', dir=root))
    try:
        for name, data in files.items():
            target = temp / name
            target.parent.mkdir(parents=True, exist_ok=True)
            write_new(target, data)
        require(validate_patch(temp)[0] == record, 'import snapshot mismatch')
        # Offline exclusively owned root: no concurrent writer allowed.
        require(not dest.exists(), 'destination appeared')
        temp.rename(dest)
    finally:
        if temp.exists():
            shutil.rmtree(temp)
    return record

def main():
    p = argparse.ArgumentParser(description=__doc__)
    sub = p.add_subparsers(dest='command', required=True)
    patch = sub.add_parser('patch'); patch.add_argument('folder')
    for cmd in ('catalog', 'check'):
        c = sub.add_parser(cmd); c.add_argument('root'); c.add_argument('--approvals', required=True)
        c.add_argument('--previous'); c.add_argument('--catalog', required=True)
    imp = sub.add_parser('import'); imp.add_argument('source'); imp.add_argument('root'); imp.add_argument('--approvals', required=True)
    a = p.parse_args()
    try:
        if a.command == 'patch':
            print(canonical(validate_patch(a.folder)[0]).decode(), end='')
        elif a.command == 'import':
            print(canonical(import_patch(a.source, a.root, a.approvals)).decode(), end='')
        else:
            catalog = build_catalog(a.root, a.approvals, a.previous)
            if a.command == 'catalog':
                write_new(a.catalog, canonical(catalog))
            else:
                require(canonical(loads(Path(a.catalog).read_bytes())) == canonical(catalog), 'catalog does not match approved library')
            print('OK: approved offline catalog; NOT runtime or redistribution approval')
    except (Invalid, OSError, KeyError, TypeError, RecursionError) as exc:
        p.exit(1, 'REJECTED: ' + str(exc) + '\n')

if __name__ == '__main__':
    main()
