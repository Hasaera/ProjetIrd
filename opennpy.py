import numpy as np

# Chemin vers le fichier .npy
file_path = '/home/hasaera/Documents/AlternanceIrd/signals.npy'

# Charger le fichier .npy
data = np.load(file_path)

# Afficher le contenu du fichier
print(data)
