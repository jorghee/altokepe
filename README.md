# :skull: <samp>Sistema Altokepe</samp>

Sistema inteligente de gestión de pedidos en restaurantes.

En esta seccion vamos a explicar el protocolo de comunicación 
y el flujo de trabajo que los clientes `Recepcionista`, 
`EstacionCocina` y `Ranking` deben seguir para interactuar 
correctamente con el servidor.

## 1. Comunicación del Servidor

El servidor opera bajo un **modelo de comunicación orientado a 
eventos** sobre TCP. Toda la comunicación se realiza mediante 
objetos JSON terminados con un carácter de nueva línea (`\n`).

El flujo para cualquier cliente es siempre el mismo:

1.  **Conexión e Identificación:** El cliente establece una 
conexión TCP con el servidor y envía un único mensaje de 
`IDENTIFICARSE`.

2.  **Recepción de Estado Inicial:** Tras una identificación 
exitosa, el servidor envía un mensaje de 
`ACTUALIZACION_ESTADO_GENERAL` que contiene el estado completo 
del sistema relevante para ese cliente. Esto permite que la 
UI se renderice por primera vez.

3.  **Comunicación en Tiempo Real:**
    *   **Cliente a Servidor:** El cliente envía **comandos** 
    (requests) para realizar acciones (ej. crear un nuevo 
    pedido).
    *   **Servidor a Cliente:** El servidor envía **eventos** 
    (notificaciones) para informar al cliente de cambios que 
    han ocurrido (ej. un plato está listo, un nuevo pedido ha 
    llegado a una estación). La UI del cliente debe reaccionar 
    a estos eventos para actualizarse sin necesidad de recargar 
    todo.

---

## :fork_and_knife: 2. Cliente: Recepcionista

### Rol en el sistema

El Recepcionista es el punto de entrada para todos los pedidos 
de los clientes. Su función principal es tomar los pedidos de 
las mesas y enviarlos al sistema.

### Conexión e Identificación

Al conectarse, el cliente Recepcionista **debe** enviar el 
siguiente mensaje para identificarse. El `id` debe ser un 
entero único que identifique a ese recepcionista en particular.

```json
{
  "comando": "IDENTIFICARSE",
  "rol": "Recepcionista",
  "id": 101 
}
```

### Acciones (Mensajes Enviados al Servidor)

El Recepcionista solo tiene una acción principal: crear un 
nuevo pedido.

#### Comando: `NUEVO_PEDIDO`

-   **Propósito:** Enviar un nuevo pedido de una mesa al 
sistema para que el Manager Chef lo gestione.
-   **Formato del Mensaje:**
    ```json
    {
      "comando": "NUEVO_PEDIDO",
      "mesa": 4,
      "id_recepcionista": 101,
      "platos": [
        { "id": 101, "cantidad": 1 },
        { "id": 201, "cantidad": 2 }
      ]
    }
    ```
-   **Campos:**
    -   `mesa`: (entero) El número de la mesa que realiza el 
    pedido.
    -   `id_recepcionista`: (entero) El ID con el que el 
    recepcionista se identificó.
    -   `platos`: (array de objetos) La lista de platos 
    pedidos. Cada objeto contiene:
        -   `id`: (entero) El ID del plato según el menú.
        -   `cantidad`: (entero) La cantidad de ese plato 
        que se ha pedido.

### Reacciones (Eventos Recibidos del Servidor)

El Recepcionista debe estar atento a las notificaciones que le 
indican cuándo un plato está listo para ser recogido y llevado 
a la mesa.

#### Evento: `PLATO_LISTO`

-   **Propósito:** Notificar al recepcionista que uno o más 
platos de un pedido están listos en el área de despacho.
-   **Formato del Mensaje:**
    ```json
    {
      "evento": "PLATO_LISTO",
      "data": {
        "id_pedido": 1,
        "mesa": 4
      }
    }
    ```
-   **Campos en `data`:**
    -   `id_pedido`: (entero largo) El ID del pedido que 
    tiene platos listos.
    -   `mesa`: (entero) El número de la mesa a la que 
    pertenece el pedido.
-   **Acción del Cliente:** La UI del recepcionista debe 
mostrar una alerta o notificación prominente, indicando que 
debe ir a buscar la comida para la mesa especificada.

---

## :man_cook: 3. Cliente: Estación de Cocina

### Rol en el sistema

Representa una estación de trabajo específica en la cocina 
(ej. "Carnes", "Guisos", "Bebidas"). Su responsabilidad es ver 
la cola de platos que le han sido asignados, prepararlos y 
notificar al sistema cuando están terminados.

### Conexión e Identificación

La identificación de una estación es **crucial** para el 
enrutamiento de platos. **Debe** especificar su 
`nombre_estacion` único.

```json
{
  "comando": "IDENTIFICARSE",
  "rol": "EstacionCocina",
  "nombre_estacion": "Carnes"
}
```
*   **Nota:** El valor de `nombre_estacion` debe coincidir 
exactamente con el campo `"estacion"` de los platos en 
`menu.json`.

### Acciones (Mensajes Enviados al Servidor)

La única acción de una estación es marcar un plato como 
finalizado.

#### Comando: `MARCAR_PLATO_TERMINADO`

-   **Propósito:** Informar al servidor que la preparación de 
una instancia de plato específica ha concluido.
-   **Formato del Mensaje:**
    ```json
    {
      "comando": "MARCAR_PLATO_TERMINADO",
      "id_pedido": 1,
      "id_instancia": 1001
    }
    ```
-   **Campos:**
    -   `id_pedido`: (entero largo) El ID del pedido al que 
    pertenece el plato.
    -   `id_instancia`: (entero largo) El ID único de la 
    **instancia** del plato que se ha terminado. Este ID es 
    recibido previamente en el evento `NUEVO_PLATO_EN_COLA`.

### Reacciones (Eventos Recibidos del Servidor)

La estación necesita saber qué platos tiene que preparar.

#### Evento: `NUEVO_PLATO_EN_COLA`

-   **Propósito:** Informar a la estación que un nuevo plato 
ha sido asignado a su cola de trabajo.
-   **Formato del Mensaje:**
    ```json
    {
      "evento": "NUEVO_PLATO_EN_COLA",
      "data": {
        "nombre": "Lomo Saltado",
        "id_pedido": 1,
        "id_instancia": 1001,
        "score": 12.5
      }
    }
    ```
-   **Campos en `data`:**
    -   `nombre`: (string) El nombre del plato a preparar.
    -   `id_pedido`: (entero largo) El ID del pedido al que 
    pertenece.
    -   `id_instancia`: (entero largo) El ID único de esta 
    instancia de plato. **Este es el ID que se debe enviar de 
    vuelta al marcarlo como terminado.**
    -   `score`: (doble) La puntuación de prioridad. Un número 
    más bajo significa mayor prioridad.
-   **Acción del Cliente:** La UI debe añadir este plato a su 
lista de "pendientes", ordenándola por `score`.

*   **Nota sobre Devoluciones:** Si el Manager Chef devuelve un 
plato a la cocina, la estación recibirá un evento 
`NUEVO_PLATO_EN_COLA` con un `score` muy bajo (ej. `-9999.0`), 
indicando que debe ser preparado con la máxima urgencia.

---

## :man_shrugging: 4. Cliente: Ranking

### Rol en el sistema

El cliente Ranking es una pantalla **de solo lectura** que 
muestra una clasificación de los platos más pedidos. Es el 
cliente más simple.

### Conexión e Identificación

Se identifica simplemente con su rol.

```json
{
  "comando": "IDENTIFICARSE",
  "rol": "Ranking"
}
```

### Acciones (Mensajes Enviados al Servidor)

**Ninguna.** El cliente Ranking no envía comandos que 
modifiquen el estado del servidor. Es un cliente pasivo.

### Reacciones (Eventos Recibidos del Servidor)

#### Evento: `ACTUALIZACION_ESTADO_GENERAL` (En la Conexión)

-   **Propósito:** Al conectarse, el servidor le enviará el 
estado inicial completo, que incluye el ranking actual.
-   **Formato del Mensaje:**
    ```json
    {
      "evento": "ACTUALIZACION_ESTADO_GENERAL",
      "data": {
        "pedidos_pendientes": [...],
        "pedidos_en_progreso": [...],
        "pedidos_terminados": [...],
        "ranking": [
          { "nombre": "Lomo Saltado", "cantidad": 5 },
          { "nombre": "Inca Kola", "cantidad": 8 },
          { "nombre": "Ají de Gallina", "cantidad": 3 }
        ],
        "menu": [...]
      }
    }
    ```
-   **Acción del Cliente:** El cliente debe extraer el array 
`ranking` del objeto `data` y mostrarlo. Los otros campos 
(`pedidos_*`, `menu`) pueden ser ignorados.

#### Nota sobre Actualizaciones en Tiempo Real

Actualmente, el servidor **no** emite eventos granulares para 
actualizar el ranking en tiempo real. El ranking solo se 
actualiza si el cliente se desconecta y se vuelve a conectar.

*   **Futura Mejora (Opcional):** Se podría implementar un 
nuevo evento `RANKING_ACTUALIZADO` en el servidor que se envíe 
cada vez que `m_conteoPlatosRanking` cambie, permitiendo una 
actualización en vivo. Sin embargo, aún está en construcción.
