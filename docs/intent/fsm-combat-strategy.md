# Statement of Intent: FSM Combat Strategy & Progressive Acceleration

- **Outcome:** Eliminar `STATE_INITIAL_TACTIC` para pasar de los 5 segundos de espera directamente a búsqueda girando sobre el eje a velocidad 100 con umbral de detección `<= 60 cm`, incorporando una rampa de aceleración progresiva y no bloqueante por distancia (`<= 30 cm`: 200, `<= 25 cm`: 220, `<= 18 cm`: 235, `<= 10 cm` o zona ciega: 255).
- **User:** El robot minisumo en combate para maximizar tracción, adherencia y velocidad de reacción.
- **Why now:** Eliminar comportamientos iniciales innecesarios, suprimir falsos ecos a más de 60 cm en el dohyo y erradicar patinadas o bloqueos de sensado generados por arranques bruscos o retardos.
- **Success:**
  1. Cumplidos los 5s de `STATE_SAFETY_DELAY`, entra en `STATE_SEARCH` girando sobre su propio eje a velocidad 100.
  2. Al detectar al oponente a `<= 60 cm`, avanza escalonando la potencia en tiempo real según la distancia sin usar ningún `delay()`.
  3. Al llegar a `<= 10 cm` o entrar en la zona ciega del sensor ultrasónico (< 2 cm), ataca al 100% (255) manteniendo la lectura de línea blanca activa en cada ciclo.
- **Constraint:** Ninguna llamada bloqueante (`delay`) en la modulación de velocidad para garantizar que la prioridad de línea blanca responda en microsegundos.
- **Out of scope:** Modificar las maniobras de escape de línea (`STATE_EVADE_BACKUP`, `STATE_EVADE_TURN`) o alterar las funciones de hardware de motores y sensores.
