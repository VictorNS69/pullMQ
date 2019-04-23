# Diseño de un sistema de cola de mensajes (_pullMQ_)
Se trata de un proyecto práctico de carácter individual cuyo plazo de entrega termina el 5 de mayo.
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
```
int createMQ(const char *cola);
int destroyMQ(const char *cola);
```
Nótese que se va a implementar un modo de acción inmediato en la operación de destrucción eliminándose directamente todos los mensajes almacenados en la cola. Se podría haber planteado un modo diferido (seleccionable mediante un parámetro de la operación) en el que se retrasase la destrucción final de la cola hasta que se hubieran leído todos los mensajes de la misma, no permitiendo, mientras tanto, añadir nuevos.

La operación de escritura/envío a la cola especifica el nombre de la cola y las características del mensaje a enviar, cuyo tamaño máximo no debe limitarse en el diseño de la práctica, devolviendo 0 si la operación se realizó satisfactoriamente y un valor negativo en caso contrario (por ejemplo, si se intenta escribir en una cola que no existe). A la hora de evaluar la práctica, no se realizarán pruebas en las que los parámetros que especifican el mensaje tengan valores extraños (por ejemplo, que el puntero al mensaje sea nulo o el tamaño menor o igual que 0), por lo que puede aplicar en estos casos patológicos el tratamiento que considere oportuno.
```
int put(const char *cola, const void *mensaje, size_t tam);
```
La operación de lectura/recepción de la cola especifica el nombre de la cola como parámetro de entrada y, a continuación dos parámetros de salida vinculados con el mensaje recibido:

- El parámetro _mensaje_, a diferencia de lo que ocurre con _read_ o _recv_, no es un puntero a un _buffer_ especificado por el módulo llamante donde se depositará el nuevo mensaje, sino que en él se devuelve el puntero al _buffer_ que esta función ha habilitado en el _heap_ para almacenar el mensaje.
- El parámetro _tam_ devuelve el tamaño del mensaje recibido.
Nótese que el módulo que ha invocado esta operación get debería llamar a _free_ cuando termine de procesar el mensaje para liberar la memoria asociada al mismo. El último parámetro especifica si la lectura es bloqueante (aspecto que se implementa en la segunda fase) o no. Nótese que una lectura no bloqueante devolverá un 0 en el parámetro _tam_ si no hay ningún mensaje en la cola. La función retornará 0 si la operación se realizó satisfactoriamente y un valor negativo en caso contrario (por ejemplo, si se intenta leer de una cola que no existe). Nuevamente, puede aplicar el tratamiento que considere oportuno en caso de que el parámetro que especifica el mensaje tenga valores _extraños_ (por ejemplo, que el puntero al mensaje sea nulo).
```
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

