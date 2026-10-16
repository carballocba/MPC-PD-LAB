# Etapa 1 — resultados, 16-10-2026

Rama local: development/pdlab-v0.3-offline-library. Base v0.2: 062ac5cec83b8ab469ade5f1df059a97cb1d2634.

- 43 tests unittest: PASS (log íntegro TEST_LOG.txt). Incluyen CLI de generación/check/rechazo de overwrite, importación con revisión externa, duplicados, JSON corrupto/NaN/keys repetidas, identidad/revisión, hashes, orden, mutación de revisión y nueva revisión conservando la anterior, rutas/symlinks/hardlinks/ejecutables/quotas, licencia vacía/referencia inválida, dependencias desconocidas, externals/assets, recursión de abstracciones y contratos Gain/Mix.
- DSP Trim y Lowpass comparado byte por byte contra originales: PASS.
- Igualdad exacta de whitelist con src/universal/manifest.c: PASS.
- Catálogo de ejemplo regenerado/verificado desde fixtures y approvals: PASS.
- git diff --exit-code contra base para src, patches, scripts, packaging, CMakeLists.txt y .github: PASS (sin cambios). Todos los cambios son archivos nuevos en cuatro directorios offline.

Fallos investigados durante desarrollo: alias symlink macOS /var en tempdir (tests usan ruta real; política de rechazo se conserva), comparación stat incluía atime alterado por lectura (ahora compara identidad/tamaño/mtime/ctime/mode/nlink), fixture de mutación requería actualizar provenance antes de probar inmutabilidad. Corregidos y suite completa repetida.

No se ejecutó Pd, native Linux/ARM ni suite DSP: no hubo cambios al runtime/build; las pruebas relevantes de esta etapa son Python offline. No hubo hardware, instalación, push ni merge. No se presenta compatibilidad ARM o funcionamiento musical nuevo como validado.

Limitaciones: fixtures solo offline, redistribución no aprobada por autoría pendiente del aviso heredado; schema limitado al perfil actual y validador de subset documentado; raíz privada sin carreras adversariales; límites de escaneo conservadores; sin sandbox ni prueba exhaustiva de sintaxis/DSP. --previous requiere catálogo baseline confiable para detectar mutación histórica.

Esperar revisión antes del motor, ABI, banco de parámetros, GUI o hardware.
