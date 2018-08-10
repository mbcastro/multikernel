
export PLATUML="java -jar /opt/plantuml/plantuml.jar"
mkdir -p doc/class-diagrams

$PLATUML doc/uml/kernel.uml && mv doc/uml/kernel.png doc/class-diagrams/kernel.png
$PLATUML doc/uml/libs.uml   && mv doc/uml/libs.png doc/class-diagrams/libs.png
$PLATUML doc/uml/nanvix.uml && mv doc/uml/nanvix.png doc/class-diagrams/nanvix.png
