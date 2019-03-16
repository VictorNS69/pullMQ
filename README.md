# pullMQ
Sistema de colas de mensajes con un diseño básico.
## Objetivo de la práctica
El objetivo principal es que el alumno pueda ver de una forma aplicada qué infraestructura de comunicación requiere la construcción de un sistema de colas de mensajes con un diseño básico (nota: puede ser conveniente que repase el material teórico de la asignatura para refrescar la terminología y los conceptos asociados a este esquema de interacción). Para ello, se plantea desarrollar un esquema de este tipo, al que denominaremos _pullMQ_, con las siguientes características específicas:

- Se utilizará un esquema con un único proceso que actúa como _broker_ proporcionando el desacoplamiento espacial y temporal ofrecido por los sistemas de colas de mensajes. El _broker_ se encargará de gestionar las colas y almacenar los mensajes dirigidos a las mismas. Gracias al desacoplamiento espacial, los procesos que usan este sistema no se necesitan conocer ni interaccionar entre sí, sino que se comunicarán únicamente con el _broker_ haciendo referencia al nombre de la cola de mensajes correspondiente. Asimismo, el almacenamiento de los mensajes de las colas por parte del _broker_ proporcionará el desacoplamiento temporal requerido.
- Se va a seguir un esquema de tipo _pull_, en el que un proceso que quiere recibir mensajes dirigidos a una cola debe pedirlos explícitamente, en lugar de ser notificado de la existencia de los mismos por parte del _broker_.
- El _broker_ gestionará los mensajes que se envían a las colas sin realizar ninguna manipulación sobre el contenido de los mismos. Las aplicaciones que usen este sistema deberían definir el formato de los mensajes enviados según su conveniencia.
- El diseño del sistema **no debe establecer ninguna limitación en el tamaño de los mensajes ni en la longitud de los nombres de las colas, ni tampoco en el número de colas que puede existir en el sistema**.

Como parte del desarrollo del proyecto, el alumno tendrá que enfrentarse al diseño del protocolo de comunicación del sistema, disponiendo de total libertad para definir todos los aspectos requeridos por el mismo: qué intercambio de mensajes se lleva a cabo en cada operación, el formato de esos mensajes, si se usan conexiones persistentes entre los procesos y el broker, etcétera.

En cuanto a las tecnologías usadas en la práctica, se programará en el lenguaje C utilizando sockets de tipo stream y se supondrá un entorno de máquinas heterogéneas.

Se van a distinguir tres fases en el desarrollo de la práctica:
- Sistema en el que las operaciones de lectura de una cola son solo no bloqueantes. Con esta fase puede alcanzarse una nota de **6 puntos**.
- Sistema que incorpora también operaciones de lectura bloqueantes, que otorga un máximo de **3 puntos adicionales**.
- Una fase final que plantea analizar e implementar aspectos de tolerancia a fallos en el sistema, que puede proporcionar el **1 punto** restante. Nótese que en las dos fases previas no se tendrá en cuenta la posibilidad de que alguno de los procesos involucrados en una comunicación se caiga, siendo en esta fase cuando se afronta este tipo de problemas.

## Autor
[Víctor Nieves Sánchez](https://twitter.com/VictorNS69)

## Licencia
[Licencia](/LICENSE).
