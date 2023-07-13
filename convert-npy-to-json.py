import numpy as np
import json

def convert_npy_to_json(npy_file, json_file):
    # Charger le fichier .npy
    data = np.load(npy_file)
    
    # Convertir les données en une structure JSON compatible
    json_data = data.tolist()
    
    # Écrire les données converties dans un fichier JSON
    with open(json_file, 'w') as file:
        json.dump(json_data, file)

# Utilisation de la fonction de conversion
npy_file = '/home/hasaera/Documents/AlternanceIrd/IRD/signals.npy'
json_file = '/home/hasaera/Documents/AlternanceIrd/IRD/signals.json'

convert_npy_to_json(npy_file, json_file)
