# PD LAB — sustitución de fixtures y procedencia

Fecha: 2026-10-16. Rama local `development/pdlab-original-fixtures`, base `562102529b893eb86febfcfbe88d2b8e4421e01b`. Sin push, merge, instalación ni acceso al MPC.

## Creación nueva documentada

Se escribieron desde cero dos grafos en esta tarea, mediante `tools/pdlab_offline/create_original_fixtures.py`. El generador no lee los patches anteriores ni utiliza sus archivos como plantilla. La procedencia identifica a **OpenAI Codex como agente generador bajo instrucción del usuario**, no como titular jurídico ni autor humano. No se afirma copyright de Bernardo, Serge, sd88me, Miller ni otra persona sobre el material nuevo.

El generador, los dos grafos, sus manifiestos/provenance y harness de pruebas son aportaciones creadas para esta tarea. Se documenta el proceso y su hash, no una auditoría legal de titularidad. Ya existía conocimiento del proyecto y sus fixtures: esto no se presenta como un proceso formal clean-room con aislamiento entre equipos, ni una garantía de originalidad jurídica. Las operaciones y fórmulas usadas son estándar.

Identidades nuevas: `org.pdlab.original-gain-mix/1` y `org.pdlab.original-lowpass/1`. Ninguna sustituye silenciosamente revisión 1 de los IDs anteriores. Los JSON PROVENANCE guardan fecha, agente, método, hash del generador, hash del grafo, componentes externos y estado de derechos.

El campo heredado `sourceCommit` usa `original-design-2026-10-16-v1` como etiqueta explícita de receta, **no como SHA Git inventado**. `source.path` apunta al generador; `source.sha256` al grafo resultante según contrato offline existente. Git registra después el commit real de incorporación; el hash del generador permite reproducir los bytes sin circularidad de un commit dentro de su propio archivo.

## Diseño musical nuevo

Ambos usan la ecuación `salida = seco + mix * (gain * procesado - seco)` por canal. Gain y Mix siguen en [0,1], por ul-gain/ul-mix, con rampas internas de 20 ms. Defaults del manifiesto 1/1 requieren que el host los envíe, como el contrato actual. No hay loadbang u objetos nuevos.

- **Stereo Gain/Mix:** procesado=seco. Ganancia efectiva `(1-mix)+mix*gain`; con Mix=0 hay bypass seco a ganancia unidad. Con Mix=1, salida=gain*entrada.
- **Stereo Lowpass:** procesado=`lop~ 900` independiente por canal; Mix=0 conserva seco, Mix=1 entrega señal filtrada escalada. Corte fijo 900 Hz, no un parámetro adicional.

Diferencia explícita frente a los fixtures anteriores: ganancia aplicada a rama wet, no atenuación final del dry/wet completo; rampas 20 ms y corte 900 Hz. No se promete equivalencia sample-exact con los efectos antiguos ni reemplazo de sus proyectos. Estas son nuevas implementaciones con la misma interfaz de controles y el mismo conjunto permitido de objetos.

## Componentes externos

Los grafos referencian objetos Vanilla ya permitidos: inlet~, outlet~, r, pack, line~, *~, +~, -~ y lop~ donde corresponde. No se copió código C de Pure Data en los grafos ni patches externos/anteriores. `sig~` permanece permitido por el perfil, pero no se usa. No jsmn, externals, samples, scripts ni abstractions externas en los nuevos paquetes.

El test usa el libpd/Pd privado existente, BSD con notices originales, sin recompilarlo. Su harness de audio crea un pequeño host de test con adc~/dac~ y la abstracción main: estos objetos **no se añaden al perfil de patches**. La predicción matemática de lop~ usa su recurrencia documentada por el código de Pd (`d_filter.c` en el pin f009fd8d7b537e209e09898d487fdf1bf547da2b) como oráculo de test, no como implementación copiada del efecto.

Herramienta offline y restricciones se reutilizan del proyecto. La plantilla MIT toma el texto estándar de [Open Source Initiative](https://opensource.org/license/mit); nombres/año quedan sin completar y el archivo dice expresamente que no concede licencia.

## Derechos y MIT

LICENSE.txt de cada fixture explica que no existe todavía una concesión acreditada. Manifiestos schema 2 usan `spdx: NOASSERTION` y `pending-rightsholder-authorization`. Eso separa el proceso de creación documentado del derecho a publicar. La plantilla MIT se entrega fuera de los fixtures y no se vincula como licencia efectiva.

Approvals nuevos: hash técnico + scope offline-fixture + licenseDecision pending-rightsholder-authorization. No son aprobación del titular. Catálogo: runtimeApproved=false, redistributionApproved=false. No hay modo de aprobar distribución automáticamente. La autorización pendiente debe identificar a la persona/entidad facultada y el alcance, vinculados a estos hashes; no basta completar placeholders arbitrariamente. Una autorización futura requerirá revisión de metadatos nueva, nunca reescritura de revisión 1.

El problema de atribución de jsmn se elimina de los **fixtures activos nuevos**; no se afirma que los derechos de todos los componentes o plugins históricos estén saneados. La publicación MIT definitiva sigue pendiente de la autorización que solicitó el usuario.

## Cuarentena y revocaciones

Se trasladó byte por byte el árbol original library-v03 a quarantine/stage1-fixtures/library-v03, junto con catálogo/approvals históricos renombrados NOT-VALID. Se retienen avisos íntegros de terceros como evidencia; no se adjudican al material nuevo. Un test compara todos esos archivos con c565aaf.

El catálogo activo contiene solo los dos IDs nuevos y registra revocaciones de los dos pares ID/revisión anteriores con sus hashes exactos. El validador de baseline admite retirada únicamente mediante registro explícito de revocación, vinculado al hash anterior; rechaza mutación silenciosa y hash de revocación incorrecto. Es un ledger técnico local, no un sistema de firma o revocación remota.

Las copias originales bajo patches/universal se mantienen intactas porque forman parte de los plugins existentes. No se carga ni redistribuye ahora ese runtime; su problema documental sigue indicado. Poner en cuarentena la biblioteca offline no cambia retroactivamente las releases existentes.

## Validación efectuada

- **51/51 tests offline PASS**: contratos previos y tests nuevos de no-concesión, MIT no confirmado rechazado, rechazo de fixtures/approvals antiguos, revocación exacta, preservación histórica y hash de generador.
- **72 casos funcionales con libpd PASS**: nueve pares Gain/Mix × sine/impulse × izquierda/derecha, 36 por efecto. Se envían controles y se espera estabilización antes de comparar con ecuación independiente.
- Error máximo absoluto frente a referencia: Gain/Mix `7.4505806e-09`; Lowpass `3.27033754e-08` (tolerancia 2e-5).
- Respuesta RMS 100 Hz / 9 kHz: Gain/Mix 1.000000 / 1.000000; Lowpass 0.994681 / 0.114058. Señal en canal contrario permanece nula. Valores finitos, endpoints y mezcla intermedia comprobados.
- **Validador C del runtime existente PASS** para objetos y cierre de ambos grafos. Se compiló un test autónomo con src/universal/manifest.c sin editarlo; no un plugin nuevo.
- **54 archivos existentes de src/patches/scripts/packaging/.github/probes idénticos** a la base Stage 2A, incluidos LICENSE del runtime, UIDs/ABI, DSP de plugins y fuentes/configuración GUI. No se recompiló ni reemplazó ningún plugin.

Un test negativo inicial dependía de `sig~` del fixture anterior; el nuevo Gain/Mix no usa ese objeto. Se corrigió la mutación del test para reemplazar un inlet~ existente por shell; el validador lo rechaza y la suite completa pasa. No se amplió la whitelist.

Los tests funcionales corren en macOS arm64 con el libpd ya disponible, tick 64 y 44.1 kHz; no constituyen validación ARMv7/MPC o real-time. Comparan comportamiento tras estabilizar rampas, no garantizan todos los transitorios de automatización, sesiones largas o múltiples instancias. No se ejecutan fixtures antiguos o patches desconocidos.

`tests/original_fixtures/run_audio_checks.py` reproduce audio; `profile_check.c` comprueba el mismo whitelist del runtime. Logs y hashes se entregan con los artefactos. No se ejecutó la suite de plugins Linux/ARM porque el runtime/build permaneció intacto; no se reclaman resultados nuevos para esas plataformas.

## Auditoría adicional

Ver `SRC_UNIVERSAL_LICENSE_AUDIT.md`: el LICENSE general de src/universal coincide también con jsmn, mientras wrapper deriva de sd88me y otros archivos declaran intención MIT. Se documentó alcance y derechos pendientes; **no se modificó ese archivo ni el runtime**. Una corrección por componente requiere fase separada autorizada.

## Pendientes para revisión

Confirmar diseño musical y procedencia del proceso nuevo; identificar titular facultado y autorización efectiva de publicación MIT; decidir futura revisión de licencia; auditar aportaciones runtime por separado. No crear otros efectos, no integrar motor ni instalar estos fixtures en MPC en esta fase.
