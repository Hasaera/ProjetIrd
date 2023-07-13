import numpy as np
import xml.etree.ElementTree as ET

# Charger le fichier NPY
data = np.load('signals.npy')

# Créer l'élément racine XML
root = ET.Element('data')

# Parcourir les données et les ajouter à l'élément racine
for item in data:
    element = ET.SubElement(root, 'item') 
    element.text = str(item) 

# Créer l'objet ElementTree
tree = ET.ElementTree(root)

# Enregistrer le contenu XML dans un fichier
tree.write('signals.xml')
