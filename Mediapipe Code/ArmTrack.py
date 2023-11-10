import cv2
import mediapipe as mp
import serial
import math
import time

mp_drawing = mp.solutions.drawing_utils
mp_pose = mp.solutions.pose
#mp_hands = mp.solutions.hands #hands can be utilized to better get finger poses

msp = serial.Serial(port='COM11',baudrate = 9600,timeout=.1)

def wristPos(rightWristX, rightWristY, rightShoulderX, rightShoulderY, leftShoulderX,rightElbowX,rightElbowY):#,rightFingerX,rightFingerY):
    #global shoulderWidth
    div = abs(rightShoulderX-leftShoulderX)


    #yRel = ((rightShoulderY - rightWristY) / div)
    #yRel = "{:.2f}".format(yRel)
    #yRel = float(yRel)

    shoulderAngX = ((rightShoulderX - rightElbowX)/ div)
    shoulderAngY = ((rightShoulderY - rightElbowY) / div)

    shoulderAng = math.atan2(shoulderAngY,shoulderAngX)
    shoulderAng = shoulderAng*180/math.pi
    shoulderAng = "{:.2f}".format(shoulderAng)
    shoulderAng = float(shoulderAng)

    #xRel = ((rightShoulderX - rightWristX) / div)
    #xRel = "{:.2f}".format(xRel)
    #xRel = float(xRel)

    elbowAngX = ((rightElbowX -rightWristX) / div)
    elbowAngY = ((rightElbowY - rightWristY) / div)

    elbowAng = math.atan2(elbowAngY,elbowAngX)
    elbowAng = elbowAng*180/math.pi
    elbowAng = "{:.2f}".format(elbowAng)
    elbowAng = float(elbowAng)


    #eeX = ((rightWristX - rightFingerX) / div)
    #eeY = ((rightWristY - rightFingerY) / div)

    #ori = math.atan2(eeY,eeX)
    #ori = "{:.2f}".format(ori)
    #ori = float(ori)


    return shoulderAng, elbowAng

def elbowConfig(rightWristY,rightElbowY):
    if rightWristY < rightElbowY:
        elbowUp = 't'
    else:
        elbowUp = 'f'
    return elbowUp

#xRel = 0
#yRel = 0

shoulderAng = 0
elbowAng = 0
elbowUp = 't'
text = ""

#shoulderWidth = input("Enter shoulder width (mm):")
#shoulderWidth = float(shoulderWidth)

cap = cv2.VideoCapture(0)

with mp_pose.Pose(min_detection_confidence=0.5, min_tracking_confidence=0.5) as pose:
    while cap.isOpened():
        ret, frame = cap.read()
        # Recolor image to RGB
        image = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
        image.flags.writeable = False

        # Make detection
        results = pose.process(image)

        # Recolor back to BGR
        image.flags.writeable = True
        image = cv2.cvtColor(image, cv2.COLOR_RGB2BGR)

        # Extract landmarks
        try:
            landmarks = results.pose_landmarks.landmark
            #hand_landmarks = results.multi_hand.landmarks
            # Get coordinates
            #leftWrist = landmarks[mp_pose.PoseLandmark.LEFT_WRIST].y
            rightWristX = landmarks[mp_pose.PoseLandmark.RIGHT_WRIST].x
            rightWristY = landmarks[mp_pose.PoseLandmark.RIGHT_WRIST].y

            rightShoulderX = landmarks[mp_pose.PoseLandmark.RIGHT_SHOULDER].x
            rightShoulderY = landmarks[mp_pose.PoseLandmark.RIGHT_SHOULDER].y

            leftShoulderX = landmarks[mp_pose.PoseLandmark.LEFT_SHOULDER].x

            rightElbowY = landmarks[mp_pose.PoseLandmark.RIGHT_ELBOW].y
            rightElbowX = landmarks[mp_pose.PoseLandmark.RIGHT_ELBOW].x

            rightFingerX = landmarks[mp_pose.PoseLandmark.RIGHT_INDEX].x
            rightFingerY = landmarks[mp_pose.PoseLandmark.RIGHT_INDEX].y

            shoulderAng, elbowAng = wristPos(rightWristX,rightWristY,rightShoulderX,rightShoulderY,leftShoulderX, rightElbowX,rightElbowY)
            #elbowUp = elbowConfig(rightWristY,rightElbowY)
            text = '%.2f' % (shoulderAng)
            text = text.ljust(8,' ')
            text = text+ '%.2f' %elbowAng
            text = text.ljust(15,' ')
            text = 's'+text;
            #msp.write(bytes('s-45.00  0.00   ','utf-8'))
            msp.write(bytes(text,'utf-8'))
            print(text)
            #time.sleep(.1)
            #print(bytes(text))

        except:
            pass

        # display info
        cv2.rectangle(image, (0, 0), (255, 73), (245, 117, 16), -1)

        cv2.putText(image, text,
                    (10, 60),
                    cv2.FONT_HERSHEY_SIMPLEX, 2, (255, 255, 255), 2, cv2.LINE_AA)

        # Render detections
        mp_drawing.draw_landmarks(image, results.pose_landmarks, mp_pose.POSE_CONNECTIONS,
                                  mp_drawing.DrawingSpec(color=(245, 117, 66), thickness=2, circle_radius=2),
                                  mp_drawing.DrawingSpec(color=(245, 66, 230), thickness=2, circle_radius=2))

        # print(results)

        cv2.imshow("Mediapipe Feed", image)

        if cv2.waitKey(10) & 0xFF == ord('q'):
            break
cap.release()
cv2.destroyAllWindows()