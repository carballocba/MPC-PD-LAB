# Auditoría separada: src/universal/LICENSE

Fecha 2026-10-16. **Solo investigación; archivo y runtime no modificados.**

`git log --follow --format=fuller -- src/universal/LICENSE` identifica incorporación publicada en `029382cc3abd32aba52f6401a937341def9d0102`, 2026-10-14 00:10:26 +10:00, identidad Git carballocba. Historia archivada `ac0e10317ec329dcaf4c2d7954338acf8a800f09` contiene el mismo archivo 25 segundos antes, con identidad Git Bernardo Carballo. No son evidencia suficiente de titularidad.

El archivo es idéntico a vendor/universal/LICENSE y al LICENSE de jsmn commit `25647e692c7906b96ffd2b05ca54c097948e879c`: blob `c84fb2e973dd885ea5fd426aedf6e5a1849feeaa`, SHA-256 `4675b94a50d2afe811c52785463c854f1156056632cce17cc7133939eac8ed55`. vendor/universal/PROVENANCE.md documenta esa fuente. La concesión y copyright de Serge corresponden al tokenizer; no se encontró evidencia que los extienda a todo src/universal.

El directorio no es un componente homogéneo:

| Archivos | Evidencia | Conclusión limitada |
|---|---|---|
| vst2_wrap.c | Encabezado declara derivación de sd88me wrapper/vst2_wrap.c, commit `208745830fc2ed90b2fd0dc480ef5c554284292f`, MIT, copyright upstream | Conservar aviso/licencia upstream; identificar modificaciones del proyecto por separado |
| manifest.c | SPDX MIT; usa jsmn.h vendorizado mediante include | Mantener licencia MIT de jsmn para tokenizer; el uso no convierte a Serge en autor de todo el validador |
| engine.c | SPDX MIT; declara reutilizar contrato privado libpd Phase 2 | Hay declaración de intención MIT, pero falta titularidad/procedencia de las aportaciones; dependencias Pd/libpd mantienen sus avisos BSD |
| manifest.h, universal.h | SPDX MIT; adaptador del proyecto | Identificador no identifica por sí solo al titular ni sanea el LICENSE de jsmn aplicado al directorio |

El plan original `docs/design/UNIVERSAL_LOADER_PROTOTYPE.md` propone MIT para código propio sujeto a autorización del propietario; no es una declaración verificable de titularidad sobre todos los archivos. `scripts/build_universal.py` copia este aviso a licenses/universal.txt y `scripts/package_universal.py` etiqueta la release MIT. Es un problema de alcance documental que alcanza paquetes existentes; no se corrige automáticamente reemplazando fixtures.

Propuesta para futura fase autorizada: inventario de componentes/derivaciones por archivo; preservar upstream MIT/BSD; obtener confirmación sobre aportaciones propias; sustituir el LICENSE ambiguo por notices con alcance correcto y licencia confirmada. No inventar nombre/año de copyright ni quitar SPDX/avisos legítimos indiscriminadamente. No cambiar ni reempaquetar binarios actuales en esta fase.

Referencias reproducibles: [archivo publicado](https://github.com/carballocba/MPC-PD-LAB/blob/029382cc3abd32aba52f6401a937341def9d0102/src/universal/LICENSE), [provenance jsmn](https://github.com/carballocba/MPC-PD-LAB/blob/029382cc3abd32aba52f6401a937341def9d0102/vendor/universal/PROVENANCE.md), [MIT jsmn](https://github.com/zserge/jsmn/blob/25647e692c7906b96ffd2b05ca54c097948e879c/LICENSE), [wrapper fuente](https://github.com/sd88me/mpc-vst-plugins/blob/208745830fc2ed90b2fd0dc480ef5c554284292f/wrapper/vst2_wrap.c).

**Resultado:** aviso general de procedencia incorrecta/no acreditada para el directorio; derechos de jsmn documentados, derechos de terceros derivados deben preservarse, titularidad de aportaciones nuevas requiere confirmación. No se aplicó corrección al runtime.
