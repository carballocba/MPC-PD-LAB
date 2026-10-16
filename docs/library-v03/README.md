# PD LAB v0.3 / Etapa 1 — biblioteca offline

Rama local `development/pdlab-v0.3-offline-library`, basada en v0.2 `062ac5cec83b8ab469ade5f1df059a97cb1d2634`. Exclusivamente herramientas Python estándar, sin dependencias nuevas, Pd, VST, UI o acceso al MPC. Esta biblioteca **no puede consumirse aún por el loader actual**: formatos separados, sin recompilarlo ni cambiarlo.

## Archivos

- `tools/pdlab_offline/manifest.schema.json`: JSON Schema draft 2020-12 cerrado, perfil actual.
- `tools/pdlab_offline/library.py`: validación semántica, inventario/hash, importación y catálogo.
- `library-v03/patches/<id>/<revision>/`: copias independientes de Trim y Lowpass con manifiesto/provenance/licencia.
- `docs/library-v03/fixture-approvals.json`: revisión técnica local del hash; no autorización humana implícita ni aprobación de redistribución.
- `docs/library-v03/example-catalog.json`: catálogo reproducible fuera del árbol de patches.
- `tests/offline_library/test_library.py`: suite sin ejecución DSP.

Ejecutar desde la raíz del repositorio:

```sh
python3 -m unittest discover -s tests/offline_library -v
python3 tools/pdlab_offline/library.py patch library-v03/patches/org.pdlab.stereo-trim/1
python3 tools/pdlab_offline/library.py check library-v03 --approvals docs/library-v03/fixture-approvals.json --catalog docs/library-v03/example-catalog.json
```

## Incorporar un patch compatible

1. Trabajar offline, en carpeta propia sin escritores concurrentes. Nunca usar una raíz de plugin instalado ni modificar originales. No extraer archivos de origen desconocido con esta herramienta: solo acepta carpetas revisadas, no ZIPs ni URLs.
2. Revisar fuente/licencia y procedencia antes de importarlo. El perfil se limita a efectos estéreo 44.1 kHz, `main.pd` y abstracciones **planas**, objetos `inlet~ outlet~ r pack line~ *~ +~ -~ sig~ lop~`; receptores `ul-gain`/`ul-mix`; sin scripts, externals, assets, mensajes de sistema ni MIDI. No ampliar el perfil para lograr que un patch pase.
3. Crear manifiesto según el esquema. Solo Gain/Mix, slots locales de fixture 0/1, [0,1], default 1, mappingVersion 1, una página Main. Son restricciones transitorias de fixtures: **no congelan ABI ni presupuesto de parámetros v0.3**. Declarar objetos realmente usados y todas las abstracciones alcanzables. Prohibido ocultar objetos no aprobados en abstracciones.
4. Incluir `PROVENANCE.json` con sourceCommit/sourcePath/sourceSha256 y cambios; sourceSha256 corresponde a los bytes de main.pd. Conservar `LICENSE.txt`. `files` enumera exactamente todos los archivos excepto manifest.json, con bytes, SHA-256 y licenseRef. Ningún ejecutable; archivos permitidos .pd, LICENSE.txt y PROVENANCE.json. En este alcance la licencia aceptada estructuralmente es MIT heredada, con autoría pendiente; no es un importador general de licencias.
5. Usar un ID estable del namespace del editor y revisión ASCII inmutable. ID con minúsculas/puntos/guiones, máximo 64 caracteres; revisión máximo 32. Probar `patch <carpeta>`. Este comando devuelve hashes pero **no aprueba** el patch.
6. Revisión separada: editor/revisor audita fuentes, cierre de dependencias y avisos. Añadir a un archivo de approvals externo una entrada id/revision/treeSha256/reviewer/scope/licenseDecision. En Etapa 1 solo se acepta `scope: offline-fixture`, `licenseDecision: inherited-notice-local-only`. No usar estas etiquetas para distribuir nuevos patches ni simular una revisión humana. Para otra licencia o distribución, detenerse y solicitar revisión/extensión explícita del contrato.
7. Crear explícitamente una raíz nueva y exclusiva de biblioteca. Importar mediante `import <carpeta-revisada> <raiz> --approvals <archivo>`. El destino será patches/id/revision. No sobrescribe revisiones existentes; copia bytes desde snapshot validado y revalida antes de publicar. No corre Pd, shell ni hooks. En macOS usar rutas reales `/private/tmp` o raíz sin ancestros symlink; `/tmp` y `/var` pueden ser alias rechazados intencionalmente.
8. Generar `catalog <raiz> --approvals <archivo> --catalog <archivo-nuevo-fuera-de-raiz>`. El archivo destino debe no existir; conservar catálogo anterior. Usar `--previous <catalogo-anterior-confiable>` para impedir mutación/eliminación de revisiones anteriores. Validar con `check` usando los mismos argumentos.
9. Archivar catálogo, approvals, fuentes y log. No instalar ni conectar al MPC. Estos pasos no convierten el paquete al formato del motor actual.

Cambiar DSP/manifiesto/licencia exige revisión nueva y hash nuevo, conservando revisión anterior. Las entradas se ordenan por id/revision para reproducibilidad; un índice nunca identifica un patch. Se permiten varias revisiones del mismo ID, no duplicados de la pareja ID/revisión. Cambio de nombre de carpeta a otro ID/revisión sin actualizar contrato se rechaza.

## Hash y revisión externa

Cada archivo se verifica contra manifest.files. manifestSha256 cubre los bytes exactos de manifest.json. treeSha256 cubre un array ordenado por ruta de registros `{path,sha256,bytes}` que incluye el manifiesto. Serialización: JSON con claves ordenadas, separadores coma/dos puntos, ensure_ascii=true, sin NaN, más LF final. No concatenación ambigua ni autohash circular. catalog/approvals permanecen fuera del árbol y no entran en el hash.

Modificar espacios del manifiesto cambia el hash del árbol. El catálogo se comprueba contra regeneración completa y approvals; una firma/hash proporcionados por un atacante no otorgan confianza. `--previous` depende de un baseline conservado por el revisor: no puede detectar una revisión histórica borrada si se omite deliberadamente ese baseline.

## Seguridad y límites

Hasta 64 archivos de 128 KiB, total 1 MiB, manifiesto 16 KiB, profundidad cuatro, 256 directorios. Los mismos límites también se aplican a la raíz completa durante escaneo: **la biblioteca de esta etapa es acotada**, no el catálogo final escalable. No se amplía el límite de Pd original. Rechaza symlinks (incluidos ancestros), hardlinks, ejecutables y archivos especiales. O_NOFOLLOW y lecturas acotadas reducen carreras; no ofrecen aislamiento frente a un escritor adversarial que cambie directorios. Mantener raíz privada y sin escritores concurrentes, también durante rename/importación. Puede quedar directorio padre vacío tras fallo de importación, nunca un patch parcialmente publicado por la ruta normal.

El validador de esquema implementa solo los keywords utilizados por el esquema cerrado, auditados en un test; **no es un motor JSON Schema general**, no resuelve referencias ni red. Un consumidor externo puede usar un validador draft 2020-12, pero no sustituye los checks semánticos/dependencias/hash. No se instaló jsonschema para esta tarea.

El parser Pd es deliberadamente conservador: no escapes, mensajes, subpatches, scripts ni formatos desconocidos; verifica objetos, receptores, cierre/recursión de abstracciones y referencias de conexiones. No prueba todos los tipos de argumentos/puertos, ciclos de señales, deadlines, crash-freedom ni comportamiento musical. Una lista de objetos no es sandbox. El patch aprobado podría bloquear/crashear un host; en esta etapa **no se ejecuta ninguno**.

Externals quedan prohibidos, incluido ARMhf: el libpd privado oculta símbolos Pd. Muestras/scripts/MIDI/Organelle mother arbitrario no se habilitan. La licencia heredada atribuida a Serge Zaitsev se conserva byte por byte y se declara pendiente de confirmar autoría de DSP. Catálogo marca `redistributionApproved:false`. El contenido no afirma nuevas garantías legales ni aprobación para instalar.

Pendientes: licencias de autor confirmadas; catálogo escalable y firma/trust policy de distribución; schema futuro de adaptadores/estados/assets; integración transaccional con motor; ABI/UID y banco de parámetros; GUI/refresco/Q-Link; ARM/runtime y hardware tras autorización. Ninguna decisión de esta etapa cambia el plugin actual.
