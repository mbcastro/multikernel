
export PLATUML="java -jar /opt/plantuml/plantuml.jar"
mkdir -p doc/class-diagrams

$PLATUML doc/uml/kernel-hal.uml         && mv doc/uml/kernel-hal.png doc/class-diagrams/kernel-hal.png
$PLATUML doc/uml/kernel-hal-mppa256.uml && mv doc/uml/kernel-hal-mppa256.png doc/class-diagrams/kernel-hal-mppa256.png
$PLATUML doc/uml/kernel.uml             && mv doc/uml/kernel.png doc/class-diagrams/kernel.png
$PLATUML doc/uml/libs-ipc-barrier.uml   && mv doc/uml/libs-ipc-barrier.png doc/class-diagrams/libs-ipc-barrier.png
#$PLATUML doc/uml/libs-ipc.uml           && mv doc/uml/libs-ipc.png doc/class-diagrams/libs-ipc.png
#$PLATUML doc/uml/nameserver.uml  && mv doc/uml/nameserver.png doc/class-diagrams/servers.name.png
#$PLATUML doc/uml/nanvix.uml && mv doc/uml/nanvix.png doc/class-diagrams/nanvix.png
