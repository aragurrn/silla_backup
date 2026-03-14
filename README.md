Este repositorio pertenece al pfg SIMOSERM, una modificación reversible para sillas de ruedas electrónicas con el objetivo de ayudar a pacientes de enfermedades como parálisis cerebral o tetraplegia, a moverse con ayuda de celadores, médicos o enfermeros en lugares como centros de educación especial, centros de mayores u hospitales. 

El repositorio contiene los contenidos necesarios para replicar el proyecto.

**IMPORTANTE** 
Si desea añadir al proyecto, colaborar o adaptarlo a sus necesidades, cree una rama y aplique los cambios sobre la misma.
Este proyecto se realiza sobre la silla de ruedas Quickie Samba Modelo 2, si se desea realizar estas modificaciones exactas sobre otra silla de ruedas, puede resultar en daños permanentes sobre la misma, por lo que se recomienda seguir el proceso de ingeniería inversa detallado en la memoria ubicada en la carpeta documentos.
El proceso de ingeniería inversa sobre un producto comercial puede provocar cambios irreversibles en el mismo. Se recomienda encarecidamente que el análisis de la silla de ruedas se realice en un entorno equipado con la instrumentación adecuada por una persona cualificada para trabajar con materiales electrónicos.

La lista de materiales utilizados se encuentra bajo la carpeta documentación de manera resumida en el archivo BOM.txt y de manera ampliada en la memoria del proyecto.

El firmware principal del proyecto se encuentra bajo la carpeta platformio/silla.
El resto de proyectos bajo la carpeta platformio son archivos de pruebas implementadas y no implementadas en el sistema.
Bajo la carpeta librerias se encuentran las librerías externas utilizadas para el desarrollo de este preoyecto. Todas ellas se pueden descargar e instalar en los entornos de arduino IDE o platformio integrado en VSCODE.

Las interfaces de movimiento remoto y monitorización implementadas en totaljsflow y grafana se encuentran en la carpeta dashboards.
Bajo esta misma carpeta se encuentran las capturas de pantalla que corroboran el funcionamiento de las interfaces.

Bajo la carpeta diagramas se encuentra el diagrama del circuito físico.

Bajo la carpeta imágenes se encuentran imágenes del proyecto.

Bajo la carpeta videos se encuentran video evidencias del funcionamiento del proyecto.

Si se desea cambiar alguno de los materiales hardware del proyecto, puede hacerse perfectamente, pero han de adaptarse los metadatos de los proyectos de platformio y adaptarse el código al hardware que se desee utilizar.

**IMPORTANTE**
Este proyecto funciona bajo las condiciones y con el hardware indicados en este repositorio y en la memoria del proyecto.
