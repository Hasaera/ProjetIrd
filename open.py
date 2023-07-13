import h5py

# Ouvrir le fichier .h5 en mode lecture
file_path = '/home/hasaera/Documents/AlternanceIrd/model.h5'
fichier = h5py.File(file_path, 'r')

# Afficher toutes les clés du fichier HDF5
def print_hdf5_keys(obj, prefix=''):
    if isinstance(obj, h5py.File):
        print(f'Keys in the root group: {list(obj.keys())}')
        prefix = '/'
    elif isinstance(obj, h5py.Group):
        print(f'Keys in the group {prefix}: {list(obj.keys())}')
        prefix += '/'

    for key in obj.keys():
        if isinstance(obj[key], h5py.Group):
            print_hdf5_keys(obj[key], prefix + key)
        else:
            print(f'{prefix}{key}')

# Afficher toutes les clés et les groupes du fichier HDF5
print_hdf5_keys(fichier)

# Fermer le fichier HDF5
fichier.close()
