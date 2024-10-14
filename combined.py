def sign_fire_face():
    import pickle
    import cv2
    import mediapipe as mp
    import numpy as np
    import serial
    from datetime import datetime
    import mediapipe
    import cvzone
    from cvzone.HandTrackingModule import HandDetector
    import serial 
    #Fire🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥🔥
    import time
    import urllib.request#defines function and_ root_url = "http://192.168.140.4"
    root_url = "http://192.168.215.4"
    def sendRequest(url):
        n = urllib.request.urlopen(url)
    def switch_case(predicted_character):
        switch = {
        'A': root_url+"/H1",
        'B': root_url+"/L1",
        'C': root_url+"/H2",
        'D': root_url+"/L2",
        'E': root_url+"/H3",
        'F': root_url+"/L3",
        'G': root_url+"/H4",
        'H': root_url+"/L4"
        }
        return switch.get(predicted_character, None)

    cap = cv2.VideoCapture(0)
    arduino_baudrate = 9600
    arduino_timeout = 1 
    frame_resizing = 0.25
    cap.set(3,1280)
    cap.set(4,720)
    fire_cascade = cv2.CascadeClassifier('D:/forGRAD/ALL/fire_detection.xml')
    #ser = serial.Serial(arduino_port, arduino_baudrate, timeout=arduino_timeout)


    model_dict = pickle.load(open('./model.p', 'rb'))
    model = model_dict['model']



    labels_dict = {0: 'A', 1: 'B', 2: 'C', 3:'D',4:'E',5:'F',6:'G',7:'H',8:'I',9:'K',10:'L',11:'O',12:'R',13:'V',14:'W',15:'Y'}

    mp_hands = mp.solutions.hands
    hands = mp_hands.Hands(static_image_mode=True, min_detection_confidence=0.8, max_num_hands=1)

    mp_drawing = mp.solutions.drawing_utils
    mp_drawing_styles = mp.solutions.drawing_styles
    cap = cv2.VideoCapture(0)

    #hands = mp_hands.Hands(static_image_mode=True, min_detection_confidence=0.3)
    def detect_fire(frame):
        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
        fire = fire_cascade.detectMultiScale(frame, 1.2, 5)
        return fire if len(fire) > 0 else []
    while True:
        ret, frame = cap.read()
        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
        fire = detect_fire(frame)
        H, W, _ = frame.shape

        if len(fire) > 0:
            classnames_fire = ['fire']

            
            print("Fire is detected")
            
                
            for (x, y, w, h) in fire:
                
                cv2.rectangle(frame, (x-20, y-20), (x+w+20, y+h+20), (255, 0, 0), 2)
                cv2.putText(frame, f'{classnames_fire[0]} ', [x + 8, y + 100],
                                    fontFace=cv2.FONT_HERSHEY_SIMPLEX, fontScale=1.5, color=(0, 0, 255), thickness=2)

                roi_gray = gray[y:y+h, x:x+w]
                roi_color = frame[y:y+h, x:x+w]
                
                
                    
            else:
                
                
                cv2.imshow('frame', frame)
            



        frame_rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)

    #sign
        results = hands.process(frame_rgb)

        if results.multi_hand_landmarks:
            for hand_landmarks in results.multi_hand_landmarks:
                mp_drawing.draw_landmarks(
                    frame,
                    hand_landmarks,
                    mp_hands.HAND_CONNECTIONS,
                    mp_drawing_styles.get_default_hand_landmarks_style(),
                    mp_drawing_styles.get_default_hand_connections_style()
                )

                data_aux = []
                x_ = []
                y_ = []

                for i in range(len(hand_landmarks.landmark)):
                    x = hand_landmarks.landmark[i].x
                    y = hand_landmarks.landmark[i].y

                    x_.append(x)
                    y_.append(y)

                for i in range(len(hand_landmarks.landmark)):
                    x = hand_landmarks.landmark[i].x
                    y = hand_landmarks.landmark[i].y
                    data_aux.append(x - min(x_))
                    data_aux.append(y - min(y_))

                x1 = int(min(x_) * W) - 10
                y1 = int(min(y_) * H) - 10

                x2 = int(max(x_) * W) - 10
                y2 = int(max(y_) * H) - 10

                prediction = model.predict([np.asarray(data_aux)])
                predicted_character = labels_dict[int(prediction[0])]

                cv2.rectangle(frame, (x1, y1), (x2, y2), (128, 0, 128), 4)
                cv2.putText(frame, predicted_character, (x1, y1 - 10), cv2.FONT_HERSHEY_SIMPLEX, 1.3, (128, 0, 128), 6, cv2.LINE_AA)
                #Send a signal to Arduino based on the predicted sign
                url = switch_case(predicted_character)
                if url:
                    sendRequest(url)
                else:
                    print("Invalid character entered")
        cv2.imshow('frame', frame)
        success, img = cap.read()
        imgS = cv2.resize(img, (0, 0), fx=frame_resizing, fy=frame_resizing)
        imgS = cv2.cvtColor(imgS, cv2.COLOR_BGR2RGB)
        cv2.imshow('Webcam',img)
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    cap.release()
    cv2.destroyAllWindows()
while(1):
    sign_fire_face()    
