
export PLATUML="java -jar /opt/plantuml/plantuml.jar"
mkdir -p doc/class-diagrams

$PLATUML doc/uml/kernel-hal.uml           && mv doc/uml/kernel-hal.png doc/class-diagrams/kernel-hal.png
$PLATUML doc/uml/kernel-hal-mppa256.uml   && mv doc/uml/kernel-hal-mppa256.png doc/class-diagrams/kernel-hal-mppa256.png
$PLATUML doc/uml/kernel.uml               && mv doc/uml/kernel.png doc/class-diagrams/kernel.png
$PLATUML doc/uml/libs-ipc-barrier.uml     && mv doc/uml/libs-ipc-barrier.png doc/class-diagrams/libs-ipc-barrier.png
$PLATUML doc/uml/libs-ipc-name.uml        && mv doc/uml/libs-ipc-name.png doc/class-diagrams/libs-ipc-name.png
$PLATUML doc/uml/libs-ipc-mailbox.uml     && mv doc/uml/libs-ipc-mailbox.png doc/class-diagrams/libs-ipc-mailbox.png
$PLATUML doc/uml/libs-ipc-portal.uml      && mv doc/uml/libs-ipc-portal.png doc/class-diagrams/libs-ipc-portal.png
$PLATUML doc/uml/libs-mm-rmem.uml         && mv doc/uml/libs-mm-rmem.png doc/class-diagrams/libs-mm-rmem.png
$PLATUML doc/uml/libs-ipc-semaphore.uml   && mv doc/uml/libs-ipc-semaphore.png doc/class-diagrams/libs-ipc-semaphore.png
$PLATUML doc/uml/libs-posix-semaphore.uml && mv doc/uml/libs-posix-semaphore.png doc/class-diagrams/libs-posix-semaphore.png
$PLATUML doc/uml/libs.uml                 && mv doc/uml/libs.png doc/class-diagrams/libs.png
$PLATUML doc/uml/nanvix.uml               && mv doc/uml/nanvix.png doc/class-diagrams/nanvix.png
