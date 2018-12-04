# import the necessary packages
import os
import numpy as np
import cv2
from picamera.array import PiRGBArray
from picamera import PiCamera
import datetime as dt
import time
import datetime
import imutils
import requests
import smtplib
from email.MIMEMultipart import MIMEMultipart
from email.MIMEText import MIMEText
from email.mime.image import MIMEImage


# send email funtion
def SendEmailIntruso():
    fromaddr = "email which will send the message"
    toaddr = "email that will receive the message"
    msg = MIMEMultipart()
    msg['From'] = fromaddr
    msg['To'] = toaddr
    msg['Subject'] = "Unkown"
     
    body = "An unkown individual has been detected at "+timestamp.strftime("%A %d %B %Y %I:%M:%S%p")
    msg.attach(MIMEText(body, 'plain'))

    attachment = open("/path/to/the/intruder/pic", "rb")

    part = MIMEImage(attachment.read())
    attachment.close()
    
    msg.attach(part)
   #This will allow only outlook emails. If you want to send using e.g. gmail,
   # you need to change this
    server = smtplib.SMTP('smtp-mail.outlook.com', 587)
    server.starttls()
    server.login(fromaddr, "you password")
    text = msg.as_string()
    server.sendmail(fromaddr, toaddr, text)
    server.quit()

#there is no label 0 in our training data so subject name for index/label 0 is empty

subjects = ["", "owners name"]

    
#load OpenCV face detector, I am using LBP which is fast
#there is also a more accurate but slow Haar classifier
# cascPath = "detectors/haarcascade_frontalface_default.xml"
# cascPath = "detectors/lbpcascade_frontalface.xml"
cascPath = "detectors/lbpcascade_frontalface_improved.xml"
face_cascade = cv2.CascadeClassifier(cascPath)
fgbg = cv2.BackgroundSubtractorMOG()

# In[3]:

#function to detect face using OpenCV
def detect_face(img):
    #convert the test image to gray image as opencv face detector expects gray images
    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)

    #Realiza a deteccao multiscale e retorna uma lista de imagens
    faces = face_cascade.detectMultiScale(gray, scaleFactor=1.1, minNeighbors=5, minSize=(21, 21))
    
    #if no faces are detected then return original img
    if (len(faces) == 0):
        return None, None
    
    #under the assumption that there will be only one face,
    #extract the face area
    (x, y, w, h) = faces[0]
    
    #return only the face part of the image
    return gray[y:y+w, x:x+h], faces[0]

# In[8]:

# function to draw rectangle on image 
# according to given (x, y) coordinates and 
# given width and heigh
def draw_rectangle(img, rect):
    (x, y, w, h) = rect
    cv2.rectangle(img, (x, y), (x+w, y+h), (0, 255, 0), 2)
    
#function to draw text on give image starting from
#passed (x, y) coordinates. 
def draw_text(img, text, x, y):
    cv2.putText(img, text, (x, y), cv2.FONT_HERSHEY_PLAIN, 1.5, (0, 255, 0), 2)

# Load the recognizer
face_recognizer = cv2.createLBPHFaceRecognizer()
# Load trained local binary pattern face data
face_recognizer.load("trainedData.yml") 

# In[9]:


# this function recognizes the person in image passed
# and draws a rectangle around detected face with name of the 
# subject
def predict(test_img):
    #make a copy of the image as we don't want to chang original image
    img = test_img.copy()
    #detect face from the image
    face, rect = detect_face(img)

    #predict the image using our face recognizer 
    label, confidence = face_recognizer.predict(face)
    #get name of respective label returned by face recognizer
    label_text = subjects[label]
    
    #draw a rectangle around face detected
    draw_rectangle(img, rect)
    #draw name of predicted person
    draw_text(img, label_text, rect[0], rect[1]-5)
    
    return img, confidence

# In[10]:

# Strating video
print("Predicting images...")

# reader = PiVideoStream(resolution=(1296, 972), framerate=24).start()
# reader.camera.rotation=180
camera = PiCamera()
camera.resolution = (960, 720)
camera.framerate = 60
camera.rotation = 0
rawCapture = PiRGBArray(camera, size=(960, 720))


# allow the camera to warmup
time.sleep(5)

# initialize the timer
#horagora = dt.datetime.now()
#print(horagora)

consec="begin"
intruder=False
colorUnknown = (0, 0, 255)
colorKnown = (0, 255, 0)
elapsed = 10
sendOnce = True
time_counter=0
recognize = False

# while True:
#     # grab the raw NumPy array representing the image, then initialize the timestamp
#     # and occupied/unoccupied text
#     img = reader.read()
for frame in camera.capture_continuous(rawCapture, format="bgr", use_video_port=True):
    # grab the raw NumPy array representing the image, then initialize the timestamp
    # and occupied/unoccupied text
    img = frame.array

    # convert to gray scale
    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)

    # Use histogram equalizer for adjusting face in different light condition
    # equ = cv2.equalizeHist(gray)

    # faces = face_cascade.detectMultiScale(
    #     equ,
    #     1.05,
    #     5,
    #     minSize=(10,10))
    faces = face_cascade.detectMultiScale(
        gray,
        scaleFactor=1.1, 
        minNeighbors=5 
        # minSize=(21,21)
    )

    found_known = False
    timestamp = datetime.datetime.now()
    ts = timestamp.strftime("%A %d %B %Y %I:%M:%S%p")
    #cv2.putText(frame, ts, (10, frame.shape[0] - 10), cv2.FONT_HERSHEY_SIMPLEX, 0.35, (0, 0, 255), 1)
    cv2.putText(img, ts, (10, img.shape[0] - 10), cv2.FONT_HERSHEY_PLAIN, 1.5, (0, 255, 0), 2)
    # Draw a rectangle around the faces
    for (x, y, w, h) in faces:
        # cv2.rectangle(img, (x, y), (x+w, y+h), (0, 255, 0), 2)

        # grab the face to predict and predict it
        face = gray[y:y + h, x:x + w]

        #predict the image using our face recognizer 
        label, confidence = face_recognizer.predict(face)

        print confidence
        print label

        #get name of respective label returned by face recognizer
        if(confidence < 70):
            label_text = subjects[label]
        else:
            label_text = "unkown"
            
        if consec == "begin" and label_text == "unkown":
            consec = label_text
            color = (0, 255, 0)
            intruder=False
            time_counter = time.time()
            
##        print time_counter
##        print time.time()

##        print (time.time()-time_counter)
        if (time.time()-time_counter)>=elapsed/2:
            color = (0, 0, 255)
        # if the prediction has been "unknown" for a sufficient number of frames,
        # then we have an intruder
        if label_text == consec and (time.time()-time_counter)>=elapsed:
            # change the color of the bounding box and text
            color = (0, 0, 255)
            intruder = True
            time_free = time.time()
            
        if intruder:
            cv2.putText(img, "INTRUDER!!!", (10, 20), cv2.FONT_HERSHEY_PLAIN, 1.5, (0, 0, 255), 2)
            cv2.putText(img, label_text, (x, y - 20), cv2.FONT_HERSHEY_PLAIN, 0.75, color, 2)
            cv2.rectangle(img, (x, y), (x + w, y + h), color, 2)
            cv2.imwrite("INTRUDER.png", img);
            if sendOnce:
            	#send email
                SendEmailIntruso()
                #send data to cloud storage
                r = requests.post('your api url', json=your data)
                print r.status_code
                print r.headers['content-type']
                print r.text
                sendOnce=False
            if label_text != consec and time.time() - time_free>=elapsed:
                consec= "begin"
                intruder=False
                color = (0, 255, 0)
                sendOnce=True
                
        if label_text == "unkown":
            cv2.putText(img, label_text, (x, y - 20), cv2.FONT_HERSHEY_SIMPLEX, 0.75, colorUnknown, 2)
            cv2.rectangle(img, (x, y), (x + w, y + h), colorUnknown, 2)
        else:
            found_known = True
            cv2.putText(img, "ACCESS GRANTED", (10, 20), cv2.FONT_HERSHEY_PLAIN, 1.5, (0,255,0), 2)
            cv2.putText(img, label_text, (x, y - 20), cv2.FONT_HERSHEY_SIMPLEX, 0.75, colorKnown, 2)
            cv2.rectangle(img, (x, y), (x + w, y + h), colorKnown, 2)

    # save image if there is any face
    #if found_known:
        #file_name = str(dt.datetime.now()) + ".jpg"
        #cv2.imwrite(file_name, img)
     #   cloudinary.uploader.upload(file_name) 

    # Display the resulting frame
    cv2.imshow('Predicting...', img)

    # print and reset the timer
    #print(dt.datetime.now() - horagora)
    #horagora = dt.datetime.now()

   	# clear the stream in preparation for the next frame
    rawCapture.truncate(0)

    if(cv2.waitKey(1)==ord('q')):
        break


# cap.release()
cv2.destroyAllWindows()