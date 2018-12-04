import os
import cv2
import numpy as np
from PIL import Image

recognizer = cv2.createLBPHFaceRecognizer()
path = 'dataset'
dirs = os.listdir(path)


#function to detect face using OpenCV

def detect_face(img, image_name):
    #convert the test image to gray image as opencv face detector expects gray images
    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)

    # Use histogram equalizer for adjusting face in different light condition
    # equ = cv2.equalizeHist(gray)

    #Carrega o detector facial do OpenCv, neste caso, LBP
    # cascPath = "detectors/haarcascade_frontalface_default.xml"
    # cascPath = "detectors/lbpcascade_frontalface.xml"
    cascPath = "detectors/lbpcascade_frontalface_improved.xml" 
    face_cascade = cv2.CascadeClassifier(cascPath)

    #Realiza a deteccao multiscale e retorna uma lista de imagens
    faces = face_cascade.detectMultiScale(gray)

    #if no faces are detected then return original img
    if (len(faces) == 0):
        return None, None

    # Draw a rectangle around the faces
    i = 0
    for (x, y, w, h) in faces:
        cv2.putText(img, str(i), (x, y - 20), cv2.FONT_HERSHEY_SIMPLEX, 0.75, (0, 255, 0), 2)
        cv2.rectangle(img, (x, y), (x+w, y+h), (0, 255, 0), 2)
        i = i + 1
    
    #display an image window to show the image 
    #cv2.imshow("Training on image..." + str(image_name), cv2.resize(img, (400, 500)))
    cv2.waitKey(1000)

    #under the assumption that there will be only one face,
    #extract the face area
    (x, y, w, h) = faces[0]

    #return only the face part of the image
    return gray[y:y+w, x:x+h], faces[0]

# In[4]:

#this function will read all persons' training images, detect face from each image
#and will return two lists of exactly same size, one list 
# of faces and another list of labels for each face
def prepare_training_data(data_folder_path):
    #------STEP-1--------
    #get the directories (one directory for each subject) in data folder
    dirs = os.listdir(data_folder_path)
    
    #list to hold all subject faces
    faces = []
    #list to hold labels for all subjects
    labels = []

    #let's go through each directory and read images within it
    for dir_name in dirs:

        print dir_name
        
        #our subject directories start with letter 's' so
        #ignore any non-relevant directories if any
        if not dir_name.startswith("s"):
            continue
            
        #------STEP-2--------
        #extract label number of subject from dir_name
        #format of dir name = slabel
        #, so removing letter 's' from dir_name will give us label
        label = int(dir_name.replace("s", ""))
        
        #build path of directory containin images for current subject subject
        #sample subject_dir_path = "training-data/s1"
        subject_dir_path = data_folder_path + "/" + dir_name
        
        #get the images names that are inside the given subject directory
        subject_images_names = os.listdir(subject_dir_path)
        
        #------STEP-3--------
        #go through each image name, read image, 
        #detect face and add face to list of faces
        for image_name in subject_images_names:
            
            #ignore system files like .DS_Store
            if image_name.startswith("."):
                continue
            
            #build image path
            #sample image path = training-data/s1/1.pgm
            image_path = subject_dir_path + "/" + image_name

            #read image
            image = cv2.imread(image_path)
            cv2.imshow("Training on image...", cv2.resize(image, (400, 500)))
                        
            #detect face
            face, rect = detect_face(image, image_name)

            if face == None:
                print image_name
                continue
            
            #------STEP-4--------
            #for the purpose of this tutorial
            #we will ignore faces that are not detected
            # if face is not None:
            #add face to list of faces
            faces.append(face)
            #add label for this face
            labels.append(label)
            
    cv2.destroyAllWindows()
    cv2.waitKey(1)
    cv2.destroyAllWindows()
    
    return faces, labels

faces, labels = prepare_training_data(path)
recognizer.train(faces, np.array(labels))
recognizer.save('trainedData.yml')

cv2.destroyAllWindows()