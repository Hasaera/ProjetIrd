import xml.etree.ElementTree as ET

# Chemin vers le fichier XML
file_path = '/home/hasaera/Documents/AlternanceIrd/IRD/signals.xml'

# Parser le fichier XML
tree = ET.parse(file_path)

# Obtenir la racine de l'arbre XML
root = tree.getroot()

# Parcourir les éléments du fichier XML
for element in root.iter():
    print(element.tag, element.text)
