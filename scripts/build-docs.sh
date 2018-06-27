mkdir -p doc/class-diagrams

 java -jar /opt/plantuml/plantuml.jar doc/uml/hal.uml && mv  doc/uml/hal.png doc/class-diagrams/microkernel.hal.png
 java -jar /opt/plantuml/plantuml.jar doc/uml/mppa256.uml && mv doc/uml/mppa256.png doc/class-diagrams/microkernel.arch.mppa256.png
 java -jar /opt/plantuml/plantuml.jar doc/uml/ipc.uml && mv doc/uml/ipc.png doc/class-diagrams/microkernel.ipc.png
 java -jar /opt/plantuml/plantuml.jar doc/uml/nameserver.uml && mv doc/uml/nameserver.png doc/class-diagrams/servers.name.png
 java -jar /opt/plantuml/plantuml.jar doc/uml/microkernel.uml && mv doc/uml/microkernel.png doc/class-diagrams/microkernel.png
