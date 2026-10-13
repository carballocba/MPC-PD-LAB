# MPC-PD-LAB fresh-install-only preflight. No target files have been changed.
[ ! -e "$SYNTHS/$SKIN" ] && [ ! -L "$SYNTHS/$SKIN" ] || die "plugin folder already exists; refusing to overwrite"
[ ! -e "$SYNTHS/.$SKIN.new" ] && [ ! -L "$SYNTHS/.$SKIN.new" ] || die "staging folder already exists"
[ ! -e "$SYNTHS/.$SKIN.old" ] && [ ! -L "$SYNTHS/.$SKIN.old" ] || die "backup folder already exists"
[ ! -e "$OLDFILE" ] && [ ! -L "$OLDFILE" ] || die "legacy plugin exists; refusing migration"
# Case-insensitive hexadecimal UID; also refuse any entry using this binary's
# basename, which upstream's legacy migration could otherwise affect.
if grep -i -E 'uid="0*4d50444c"|mpc_pd_delay\.so' "$SETTINGS" >/dev/null; then
    die "MPDL UID or binary already registered; resolve collision before installing"
fi
