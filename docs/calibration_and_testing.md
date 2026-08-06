# Guía de Calibración y Pruebas Físicas (Motores Amarillos TT)

Esta guía detalla las variables clave del firmware que deben calibrarse y los escenarios de prueba físicos para validar el comportamiento del robot minisumo autónomo usando motores reductores amarillos de plástico (TT Motors).

---

## 1. Variables Clave a Calibrar

Debido a que los motores amarillos poseen mayor inercia mecánica, menor torque inicial y suelen utilizarse con ruedas de goma más grandes (65mm), es crucial ajustar las siguientes variables en `src/config.h`:

| Variable | Valor Inicial | Descripción y Criterio de Ajuste |
| :--- | :---: | :--- |
| `RECOVERY_BACKUP_MS` | `250` | **Tiempo de retroceso al detectar el borde.** Con motores TT esto equivale a unos ~8-10 cm reales de desplazamiento. Si el robot retrocede muy poco y queda sobre la línea, subir a `300`. Si retrocede demasiado y corre riesgo de caerse por el extremo opuesto, bajar a `200`. |
| `RECOVERY_MS` | `450` | **Tiempo total de evasión (retroceso + giro).** La fase de giro dura `RECOVERY_MS - RECOVERY_BACKUP_MS` (200ms por defecto). Debido al bajo torque de los motores TT para derrapar ruedas de 65mm, es probable que necesites subir este valor a `550` o `600` para que logre girar un ángulo de escape útil (~90° a 135°). |
| `RECOVERY_SPIN_SPEED`| `180` | **Velocidad de giro durante la evasión.** Si al robot le cuesta vencer la fricción para girar sobre su propio eje, subir esta velocidad a `255` (máximo). |
| `SEARCH_SPEED` | `120` | **Velocidad de giro en búsqueda.** Si escuchas un zumbido agudo pero el robot no se mueve (los motores no vencen la fricción estática), incrementar este valor a `150` o `160`. |
| `OPENING_MOVE_MS` | `400` | **Duración del barrido táctico inicial.** Ajustar para lograr que el robot barra un ángulo inicial óptimo (~45° a 90°) al inicio del combate antes de pasar a búsqueda general. |
| `ATTACK_DISTANCE` | `50` | **Umbral de detección del ultrasónico (cm).** Validar en el dohyō que no detecte rebotes del piso ni a personas externas al combate. |

---

## 2. Escenarios de Prueba en Físico

Realizar estas pruebas de forma secuencial antes de competir:

### Test 1: Torque Mínimo (Evitar motores trabados)
* **Objetivo:** Verificar que el robot no se quede estático zumbando en modo búsqueda.
* **Procedimiento:** Enciende el robot, deja pasar los 5 segundos de seguridad y observa el estado de `SEARCH`.
* **Resultado Esperado:** El robot debe comenzar a girar sobre su eje de manera fluida a velocidad `SEARCH_SPEED`.
* **Acción Correctiva:** Si solo zumba, incrementa `SEARCH_SPEED` en `src/config.h`.

### Test 2: El Suicida (Validar Evasión de Borde)
* **Objetivo:** Asegurar que el robot nunca se caiga solo al llegar a la línea blanca.
* **Procedimiento:** Coloca al robot en la zona negra apuntando directamente hacia el borde a unos 10-15 cm de distancia.
* **Resultado Esperado:** 
  1. Al pisar la línea blanca con cualquiera de los sensores TCRT5000, el robot debe frenar inmediatamente.
  2. Debe retroceder de manera recta despejándose del peligro.
  3. Debe girar hacia el centro del dohyō (sentido contrario al sensor que detectó la línea) y reanudar la búsqueda.
* **Acción Correctiva:**
  * Si cae antes de reaccionar: los sensores o el debounce están lentos.
  * Si retrocede muy poco: subir `RECOVERY_BACKUP_MS`.
  * Si no llega a girar lo suficiente: subir `RECOVERY_MS` o `RECOVERY_SPIN_SPEED`.

### Test 3: El Fantasma (Validar Persistencia de Ataque)
* **Objetivo:** Evitar que el robot corte un ataque (`CHARGE`) por falsos negativos del ultrasónico (pérdida momentánea de señal).
* **Procedimiento:** Coloca un objeto (caja o el otro robot) enfrente para que salte a `CHARGE`. Retira el objeto rápidamente mientras avanza.
* **Resultado Esperado:** El robot debe continuar cargando hacia adelante por exactamente `PERSIST_MS` (200ms) antes de darse cuenta de que perdió al objetivo y volver al estado `SEARCH`.
* **Acción Correctiva:** 
  * Si ante cualquier perturbación corta el ataque y se pone a girar: subir `PERSIST_MS`.
  * Si sigue de largo y se cae del dohyō tras quitar el objeto: bajar `PERSIST_MS`.

### Test 4: Homologación de Seguridad (Countdown)
* **Objetivo:** Cumplir estrictamente con el reglamento oficial de la competencia.
* **Procedimiento:** Coloca el robot en el dohyō, enciende el switch principal y presiona el botón de inicio.
* **Resultado Esperado:** 
  1. Durante los primeros 5 segundos, el LED de estado debe parpadear rápidamente y los motores deben estar en modo `BRAKE` (completamente bloqueados).
  2. Si se le pasa un objeto o línea blanca cerca en este tiempo, el robot **no debe reaccionar**.
  3. Transcurridos los 5 segundos, debe iniciar inmediatamente con el `OPENING_MOVE`.
