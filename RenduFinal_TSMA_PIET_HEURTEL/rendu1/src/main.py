import sys
from os import listdir
from os.path import isfile, join
import csv

# mfcc_size = 22

def computeScore (mfcc1, mfcc2):        # retourne le score (différence) entre deux mfcc
    D=0.0
    if len(mfcc1) != len(mfcc2) :
        print("error")
        return 
    for i in range (len(mfcc1)) :
        D += abs(mfcc1[i]-mfcc2[i])
    return D



def mfcc_read(file) :                   # retourne une liste contenant les valeurs du fichier mfcc d'entrée
    test=open(file, "r")
    i=test.readlines()[0]
    test.close()

    cpt=0
    txt = []
    nb = ""
    for car in i :
        if (car != " "):
            nb += car
        else :
            txt = txt + [float(nb)]
            cpt += 1 
            nb = ""
    return txt



def eligible_files(folder):          # retourne une liste contenant tous les fichiers du répertoire de format mfcc
    dir = [f for f in listdir(folder) if isfile(join(folder, f))]
    tab = []
    for f in dir :
        ext = ""
        for i in range(len(f)-5, len(f)):
            ext += f[i]
        if ext == ".mfcc" :
            tab += [folder+f]
    return tab



def mfcc_files(eligible_files):         # retourne une liste contenant les données de tous les fichiers mfcc
    tab = []
    for f in eligible_files :
        tab += [mfcc_read(f)]
    return tab



def nearest(f, files):                      # retourne le fichier du répertoire files le plus proche du fichier f
    mfcc = mfcc_read(f)
    best_file = files[0]
    best = computeScore(mfcc, mfcc_read(files[0]))

    for file in files :

        mfcc2 = mfcc_read(file)
        if(computeScore(mfcc, mfcc2)<best):
            best = computeScore(mfcc, mfcc2)
            best_file = file
    return best_file



def id_from_filename(filename):         # retourne l'id d'un fichier d'après son nom (ne fonctionne que dans le cas présent, où l'id est compris dans le nom du fichier)
    slash=0
    point=0
    i=0
    for c in filename :
        i+=1
        if c== '/' :
            slash = i
    newName = filename[slash:]
    i=0
    for c in newName :
        i+=1
        if c == '.' :
            newName = newName[:i-1]
    return newName




if len(sys.argv) != 6 :
    sys.exit("Usage : <test folder> <train folder> <test csv> <train csv> <out csv>")

folder1 = sys.argv[1]
folder2 = sys.argv[2]
csv_test = sys.argv[3]
csv_train = sys.argv[4]
csv_out = sys.argv[5]

files1 = eligible_files(folder1)
files2 = eligible_files(folder2)

trainDico = {}

extension = ".wav.mfcc"         # changer cette ligne selon l'extension des fichiers

with open(csv_train, newline='') as csvIn:
    spamreader = csv.reader(csvIn, delimiter=',', quotechar='|')
    for row in spamreader :
        trainDico[row[0]] = row[1]          # on crée un dictionnaire qui attribue à chaque élément du csv train le genre correspondant
        

with open(csv_out, 'w', newline='') as csvOut:
    spamwriter = csv.writer(csvOut, delimiter=',',
                            quotechar='|', quoting=csv.QUOTE_MINIMAL)

    spamwriter.writerow(["track_id"] + ["genre_id"])    # en-tête

    with open(csv_test, newline='') as csvTest:
        spamreader = csv.reader(csvTest, delimiter=',', quotechar='|')

        for id in spamreader :              # pour chaque id du csv test 
            if(id[0] != "track_id"):
                best_file = nearest(folder1+id[0]+extension, files2)    # on regarde quel fichier du dossier d'entraînement est le plus proche
                spamwriter.writerow([id[0]] + [trainDico[id_from_filename(best_file)]]) # on écrit dans le fichier de destination l'id actuel et le genre du fichier trouvé ci-dessus
        