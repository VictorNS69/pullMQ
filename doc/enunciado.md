# Diseño de un sistema de cola de mensajes (_pullMQ_)
Se trata de un proyecto práctico de carácter individual.
## Objetivo de la práctica
El objetivo principal es que el alumno pueda ver de una forma aplicada qué infraestructura de comunicación requiere la construcción de un sistema de colas de mensajes con un diseño básico (nota: puede ser conveniente que repase el material teórico de la asignatura para refrescar la terminología y los conceptos asociados a este esquema de interacción). Para ello, se plantea desarrollar un esquema de este tipo, al que denominaremos _pullMQ_, con las siguientes características específicas:
- Se utilizará un esquema con un único proceso que actúa como _broker_ proporcionando el desacoplamiento espacial y temporal ofrecido por los sistemas de colas de mensajes. El _broker_ se encargará de gestionar las colas y almacenar los mensajes dirigidos a las mismas. Gracias al desacoplamiento espacial, los procesos que usan este sistema no se necesitan conocer ni interaccionar entre sí, sino que se comunicarán únicamente con el _broker_ haciendo referencia al nombre de la cola de mensajes correspondiente. Asimismo, el almacenamiento de los mensajes de las colas por parte del _broker_ proporcionará el desacoplamiento temporal requerido.
- Se va a seguir un esquema de tipo pull, en el que un proceso que quiere recibir mensajes dirigidos a una cola debe pedirlos explícitamente, en lugar de ser notificado de la existencia de los mismos por parte del _broker_.
- El _broker_ gestionará los mensajes que se envían a las colas sin realizar ninguna manipulación sobre el contenido de los mismos. Las aplicaciones que usen este sistema deberían definir el formato de los mensajes enviados según su conveniencia.
- El diseño del sistema **no debe establecer ninguna limitación en el tamaño de los mensajes ni en la longitud de los nombres de las colas, ni tampoco en el número de colas que puede existir en el sistema**.

Como parte del desarrollo del proyecto, el alumno tendrá que enfrentarse al diseño del protocolo de comunicación del sistema, **disponiendo de total libertad para definir todos los aspectos requeridos por el mismo**:
qué intercambio de mensajes se lleva a cabo en cada operación, el formato de esos mensajes, si se usan conexiones persistentes entre los procesos y el _broker_, etcétera.

En cuanto a las tecnologías usadas en la práctica, se programará en el lenguaje C utilizando sockets de tipo stream y se supondrá un entorno de máquinas heterogéneas.

Se van a distinguir tres fases en el desarrollo de la práctica:
- Sistema en el que las operaciones de lectura de una cola son solo no bloqueantes. Con esta fase puede alcanzarse una nota de **6 puntos**.
- Sistema que incorpora también operaciones de lectura bloqueantes, que otorga un máximo de **3 puntos** adicionales.
- Una fase final que plantea analizar e implementar aspectos de tolerancia a fallos en el sistema, que puede proporcionar el **1 punto** restante. Nótese que en las dos fases previas no se tendrá en cuenta la posibilidad de que alguno de los procesos involucrados en una comunicación se caiga, siendo en esta fase cuando se afronta este tipo de problemas.

## API ofrecido a las aplicaciones
En esta sección, se describen las 4 operaciones que se les proporcionan a las aplicaciones (están declaradas en el fichero _pullMQ.h_, que no se puede modificar y que está almacenado en el directorio correspondiente a la biblioteca (_libpullMQ_) y accesible desde el directorio del programa de test de la práctica mediante el uso de un enlace simbólico.

Las operaciones de creación y destrucción especifican únicamente el nombre de la cola, cuya longitud no debe limitarse en el diseño de la práctica, devolviendo 0 si la operación se realizó satisfactoriamente y un valor negativo en caso contrario (por ejemplo, si se intenta crear una cola que ya existe o destruir una no existente). Con respecto a qué caracteres pueden aparecer en el nombre de las colas, el enunciado no plantea ninguna restricción, pero para la evaluación se usarán identificadores alfanuméricos.
```c
int createMQ(const char *cola);
int destroyMQ(const char *cola);
```
Nótese que se va a implementar un modo de acción inmediato en la operación de destrucción eliminándose directamente todos los mensajes almacenados en la cola. Se podría haber planteado un modo diferido (seleccionable mediante un parámetro de la operación) en el que se retrasase la destrucción final de la cola hasta que se hubieran leído todos los mensajes de la misma, no permitiendo, mientras tanto, añadir nuevos.

La operación de escritura/envío a la cola especifica el nombre de la cola y las características del mensaje a enviar, cuyo tamaño máximo no debe limitarse en el diseño de la práctica, devolviendo 0 si la operación se realizó satisfactoriamente y un valor negativo en caso contrario (por ejemplo, si se intenta escribir en una cola que no existe). A la hora de evaluar la práctica, no se realizarán pruebas en las que los parámetros que especifican el mensaje tengan valores extraños (por ejemplo, que el puntero al mensaje sea nulo o el tamaño menor o igual que 0), por lo que puede aplicar en estos casos patológicos el tratamiento que considere oportuno.
```c
int put(const char *cola, const void *mensaje, size_t tam);
```
La operación de lectura/recepción de la cola especifica el nombre de la cola como parámetro de entrada y, a continuación dos parámetros de salida vinculados con el mensaje recibido:

- El parámetro _mensaje_, a diferencia de lo que ocurre con _read_ o _recv_, no es un puntero a un _buffer_ especificado por el módulo llamante donde se depositará el nuevo mensaje, sino que en él se devuelve el puntero al _buffer_ que esta función ha habilitado en el _heap_ para almacenar el mensaje.
- El parámetro _tam_ devuelve el tamaño del mensaje recibido.
Nótese que el módulo que ha invocado esta operación get debería llamar a _free_ cuando termine de procesar el mensaje para liberar la memoria asociada al mismo. El último parámetro especifica si la lectura es bloqueante (aspecto que se implementa en la segunda fase) o no. Nótese que una lectura no bloqueante devolverá un 0 en el parámetro _tam_ si no hay ningún mensaje en la cola. La función retornará 0 si la operación se realizó satisfactoriamente y un valor negativo en caso contrario (por ejemplo, si se intenta leer de una cola que no existe). Nuevamente, puede aplicar el tratamiento que considere oportuno en caso de que el parámetro que especifica el mensaje tenga valores _extraños_ (por ejemplo, que el puntero al mensaje sea nulo).
```c
int get(const char *cola, void **mensaje, size_t *tam, bool blocking);
```
Nótese que se ha optado por no incorporar operaciones para que una aplicación inicie o finalice su interacción con el sistema de colas. Si en el diseño de su práctica requiere que alguna acción se realice solo una vez al principio puede llevarla a cabo en la primera llamada a una de estas operaciones.
## Arquitectura software del sistema
Hay dos partes claramente diferenciadas en el sistema:
- El _broker_ (fichero fuente _broker/broker.c_) que incluye toda la funcionalidad del sistema, implementando las operaciones descritas en el apartado previo y gestionando las colas de mensajes. Recibe como argumento el número del puerto por el que prestará servicio.
- La biblioteca (fichero fuente _libpullMQ/libpullMQ.c_) que usan los procesos que quieren interaccionar con el _broker_, ofreciéndoles las 4 funciones explicadas en la sección anterior. Este módulo recibirá la dirección del _broker_ como dos variables de entorno:
  - _BROKER_HOST_: **nombre** de la máquina donde ejecuta el _broker_.
  - _BROKER_PORT_: número de puerto TCP por el que está escuchando.
Para facilitar la reutilización de código entre ambos módulos, se incluyen los ficheros _comun.c_ y _comun.h_, que están presentes en los directorios de ambos módulos (_broker_ y _libpullMQ_, respectivamente) mediante el uso de enlaces simbólicos (**nota**: asegúrese de que durante la manipulación de los ficheros de la práctica no pierde por error estos enlaces), donde puede incluir funcionalidad común a ambos módulos si lo considera oportuno.
  
En el directorio _test_ se encuentra un programa que usa la biblioteca (_libpullMQ.so_) y que ofrece una interfaz de texto para solicitar las 4 operaciones que proporciona el sistema.

## Ejecución de pruebas del sistema
Para probar la práctica, debería, en primer lugar, arrancar el _broker_ especificando el puerto de servicio:
```sh
triqui3: cd broker
triqui3: make
triqui3: ./broker 12345
```
A continuación, puede arrancar el programa _test_ en la misma máquina o en otras:
```sh
triqui4: cd test
triqui4: make
triqui4: export BROKER_PORT=12345
triqui4: export BROKER_HOST=triqui3.fi.upm.es
triqui4: ./test
```
## Descripción de la primera fase
Esta primera versión permite obtener una nota máxima de **6** en la práctica. En esta fase, no se implementarán las lecturas bloqueantes, ni se tendrá en cuenta la posible caída de los procesos mientras están involucrados en una comunicación.

Por lo que se refiere al _broker_, se trata del clásico servidor de tipo stream que debe gestionar una estructura de datos para almacenar el estado de las colas y que puede recibir mensajes vinculados con las 4 operaciones anteriormente explicadas:
- Creación de una cola: añadiría una nueva entrada a la estructura de datos de las colas.
- Escritura de un mensaje: guardaría el mensaje en la cola correspondiente.
- Lectura de un mensaje: obtendría el primer mensaje en orden de llegada de la cola correspondiente. En caso de no haber ninguno, se responde indicando esta circunstancia.
- Destrucción de una cola: eliminaría esa entrada de la estructura de datos de las colas, borrando todos los mensajes presentes en la misma.
Para facilitar el desarrollo de la práctica, se recomienda que el proceso _broker_ sea de **tipo secuencial** para eliminar los problemas de sincronización que se producirían en una versión concurrente.

Con respecto a la biblioteca (_libpullMQ.so_), se comporta como el típico cliente de sockets _stream_, encargándose, básicamente, de crear los mensajes de protocolo asociados a las cuatro operaciones, enviarlos y recibir las respuestas. Asismimo, tendrá que realizar la gestión de memoria dinámica requerida por los mensajes.
## Descripción de la segunda fase
Esta versión puede otorgar hasta **3** puntos adicionales. En esta fase hay que incluir la lectura bloqueante que requiere que el _broker_ almacene información de qué procesos han solicitado una operación de lectura bloqueante sobre una cola estando esta vacía para poder enviarles los mensajes en orden de llegada, tanto en lo que se refiere a los mensajes como a los procesos, cuando estos lleguen.

En caso de que se destruya una cola que tiene pendientes lectores bloqueados, hay que asegurarse de que esos lectores se desbloqueen y la llamada get en la que estaban bloqueados devuelva un error (igual que lo habría hecho si se hubiera detectado al principio que la cola no existe).

## Descripción de la tercera fase
Esta versión puede otorgar hasta **1** punto adicional. Esta fase conlleva un esfuerzo de desarrollo de código bastante limitado, pero requiere realizar cierta investigación sobre cómo afectan los posibles problemas de comunicación a los programas que usan sockets _stream_, un asunto que, en algunas circunstancias, no es de todo intuitivo.

En el caso de la biblioteca, ante cualquier problema de comunicación, la función correspondiente (crear o destruir cola, así como leer y escribir en una cola) debe devolver un valor negativo.

En cuanto al _broker_, ante un problema en el envío o recepción de mensajes, el proceso deberá seguir atendiendo a otros clientes. Nótese que es crítico que este proceso siga funcionando bajo cualquier circunstancia.

Téngase en cuenta que el protocolo TCP subyacente asegura la correcta transmisión si ninguno de los procesos involucrados se cae, pero, en caso de caída de un nodo, pueden surgir distintos escenarios, dependiendo, entre otras cosas, de cómo se haya implementado el protocolo de comunicación.

A la hora de afrontar este apartado, vamos a distinguir entre tres escenarios con problemas de sincronización debido a la caída de un cliente interaccionando con el _broker_:
- Cuando un cliente envía cualquier petición, exceptuando una lectura, y se cae justo después de hacerlo sin esperar a recibir la respuesta. Esta circunstancia es relativamente improbable puesto que el intervalo que hay entre esos dos eventos es muy pequeño.
- Cuando un cliente está esperando por un mensaje solicitado mediante una lectura bloqueante se cae. Este caso sí es bastante probable puesto que un cliente podría estar esperando por un mensaje durante un tiempo ilimitado.
- Cuando un cliente envía una lectura no bloqueante o una bloqueante que no se queda a la espera, y se cae justo después de hacerlo sin recibir la respuesta. Esta circunstancia vuelve a ser muy poco probable como en el primer escenario.

En cuanto al primer escenario, compruebe qué ocurre con el _broker_ que ha implementado cuando se da esa circunstancia, tanto si el _broker_ y el cliente están en la misma máquina como en distinta, y asegúrese de que el _broker_ no se cae en ninguna circunstancia, manteniendo el servicio a otros clientes. Por lo que se refiere al tratamiento propuesto, se va a dar por completada la operación aunque el cliente se haya caído justo después de pedirla pero antes de recibir la confirmación. Lo único que hay que asegurar es que el _broker_ no se cae en ninguna circunstancia.

Con respecto al segundo escenario, que sí es altamente probable, realice las pruebas pertinentes, tanto en local como en remoto, para comprobar qué sucede con su versión del _broker_ cuando intenta enviar un mensaje a un cliente que en su momento estaba bloqueado en una lectura y, posteriormente, se cayó. El código del _broker_ que envía el mensaje recién recibido al primer proceso lector en espera debe ser capaz de detectar esta circunstancia (deberá cambiar la versión de la fase 2 si no se detecta esta circunstancia) y, en caso de error al intentar enviárselo, debe probar con los sucesivos lectores, almacenándose el mensaje en la estructura de datos si no ha podido ser entregado.

Por lo que se refiere al tercer escenario, este enunciado no va a fijar una estrategia (y, por tanto, no será tenido en cuenta en la evaluación). Se podría optar por implementar un tratamiento similar al primero (es decir, dar por entregado el mensaje aunque el cliente se haya caído antes de recibirlo; al fin y al cabo, aunque lo hubiese recibido, el cliente podría caerse justo después del _receive_) o usar un tratamiento similar al segundo escenario.

Téngase en cuenta que un sistema real de colas de mensajes admite muchas modalidades en cuanto a la confirmación de las entregas de mensajes, existiendo incluso opciones de confirmar el mensaje cuando el cliente ya lo ha procesado (lo que en algunos sistemas se denomina confirmación _manual_).

## Ficheros a entregar
- `autores` Fichero con los datos de los autores:
```txt
DNI APELLIDOS NOMBRE MATRÍCULA
```
- `memoria.txt` Memoria de la práctica. En ella se deben comentar los aspectos del desarrollo de su práctica que considere más relevantes. Asimismo, puede exponer los comentarios personales que considere oportuno.
- `broker/broker.c` Código del _broker_.
- `broker/comun.h` Fichero de cabecera donde puede incluir, si lo precisa, definiciones comunes a los dos módulos, es decir, al _broker_ y a la biblioteca.
- `broker/comun.c` Fichero donde puede incluir, si lo precisa, implementaciones comunes a los dos módulos, es decir, al _broker_ y a la biblioteca.
- `libpullMQ/libpullMQ.c` Código de la biblioteca.
