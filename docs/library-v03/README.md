# Biblioteca offline — fixtures originales nuevos

Esta rama sustituye únicamente la biblioteca offline de ejemplo, no los patches ni plugins ya usados por el runtime.

Biblioteca activa:

- `org.pdlab.original-gain-mix`, revisión 1: Stereo Gain/Mix.
- `org.pdlab.original-lowpass`, revisión 1: Stereo Lowpass, 900 Hz fijos.

Manifiestos offline **schemaVersion 2**, perfil de objetos sin cambios, dos controles Gain/Mix [0,1] con defaults 1, sin ABI nueva. `NOASSERTION` es sentinel de ausencia de concesión, no una licencia SPDX otorgada. La plantilla MIT está en `docs/original-fixtures/MIT-DRAFT.txt`, pendiente de autorización del titular. Catálogo marca runtimeApproved=false y redistributionApproved=false; approvals solo documentan comprobaciones técnicas offline, no aprobación humana/legal.

Los fixtures antiguos y sus avisos/manifiestos exactos están en `quarantine/stage1-fixtures`, fuera de la raíz activa. Los approvals y catálogo históricos conservados allí no son válidos para admisión. `docs/library-v03/revocations.json` liga las retiradas a los hashes históricos. No modificar una revisión inmutable ni actualizar su aprobación silenciosamente.

Desde la raíz:

```sh
python3 -m unittest discover -s tests/offline_library -v
python3 tools/pdlab_offline/library.py check library-v03 --approvals docs/library-v03/fixture-approvals.json --catalog docs/library-v03/example-catalog.json --previous quarantine/stage1-fixtures/historical-catalog-NOT-VALID.json --revocations docs/library-v03/revocations.json
python3 tests/original_fixtures/run_audio_checks.py --libpd work/universal-gui-native/libpd --out ../fresh-audio-fixture-check
```

La última orden requiere el libpd existente y compilador C; no recompila libpd ni plugins. Ejecuta solo los dos grafos nuevos con un adaptador de test adc~/dac~ fuera del perfil. Los manifiestos offline no son directamente consumibles por el VST actual.

Para generar catálogo nuevo: `catalog <raiz> --approvals <archivo> --catalog <archivo-nuevo>`; añadir `--previous`/`--revocations` si se retiran entradas de baseline confiable. Revocación debe coincidir con id/revisión/treeSha256 del baseline y no admite que la identidad revocada continúe activa. Importación sigue siendo carpeta revisada, root privado/quiescente y sin sobrescritura. No importar cuarentena ni ejecutar scripts externos.

Los límites anteriores se mantienen: solo los diez objetos Vanilla existentes, receptores ul-gain/ul-mix, .pd planos y metadatos permitidos; sin externals/assets/scripts/MIDI; 64 archivos, 128 KiB/archivo, 1 MiB total y manifiesto 16 KiB. El parser no es sandbox. El esquema/validador no concede derechos y no permite cambiar NOASSERTION por MIT para pasar tests.

Antes de una publicación MIT: identificar a quien puede conceder derechos, obtener autorización vinculada a los hashes, confirmar aviso y alcance, y preparar nueva revisión/schema/approvals revisados. No existe opción automática de distribución en esta etapa. Ver `docs/original-fixtures/REPORT.md` para procedencia, pruebas y decisiones pendientes.
