#include "krembot.ino.h"
#include "math.h"

CVector2 pos;
CDegrees degreeX;
CVector2 home_position;
void foraging_controller_2_controller::setup() {
    krembot.setup();
    writeTeamColor();
    teamName = "foraging_controller_2_controller";
    LOG << foragingMsg.ourColor << std::endl;
    // init variables
    pos = posMsg.pos;
    degreeX = posMsg.degreeX;
    home_position = foragingMsg.homeLocation;
    std::string oppColor = foragingMsg.opponentColor;
    opponentColor = convertStringToColor(oppColor);
    stuck = false;
    randomTurn = false;
}

void foraging_controller_2_controller::loop() {
    krembot.loop();
    pos = posMsg.pos;
    degreeX = posMsg.degreeX;
    home_position = foragingMsg.homeLocation;
    switch (state) {
        case State::move: {
            Colors colorFront = krembot.RgbaFront.readColor();
            Colors colorFrontLeft = krembot.RgbaFrontLeft.readColor();
            Colors colorFrontRight = krembot.RgbaFrontRight.readColor();
            float distance = krembot.RgbaFront.readRGBA().Distance;
            if (!stuck && !hasFood && ((colorFront == opponentColor)  ||
                (colorFrontLeft ==  opponentColor) || (colorFrontRight ==  opponentColor))) {
                int random = rand() % 10;
                if (random != 0){
                    sandTimer.start(100);
                    krembot.Base.stop();
                    state = State::block;
                    break;
                }
            }
            if ((distance < 20) || (colorFront != Colors::None)) {
                if(hasFood){
                    stuck = true;
                }
                sandTimer.start(200);
                state = State::turn;
            } else {
                if(!hasFood){
                    if(!randomTurn){
                        sandTimer.start(1700);
                        randomTurn = true;
                    }
                    if(sandTimer.finished()){
                        sandTimer.start(200);
                        randomTurn = false;
                        state = State::turn;
                    }
                }
                if(stuckTimer.finished()){
                    stuck = false;
                }
                krembot.Base.drive(100, 0);
            }
            if (hasFood && !stuck){
                state = State::turn;
            }
            break;
        }

        case State::turn: {
            //WE NEED TO CALCULATE THE DEGREE IN WHICH THE ROBOT WILL MOVE TO HOME
            if (hasFood && !stuck){
                CDegrees angle = calculateDegreeHome();
                if (got_to_orientation(angle)) {
                    krembot.Base.stop();
                    state = State::move;
                } else {
                    int res = (angle - degreeX).UnsignedNormalize().GetValue();
                    if (res>50) { krembot.Base.drive(0, 50);
                        krembot.Base.drive(0, 50);
                    } else if (res>25) {
                        krembot.Base.drive(0, 25);
                    } else if (res>5) {
                        krembot.Base.drive(0, 5);
                    } else {
                        krembot.Base.drive(0, 1);
                    }
                }
            } else {
                // if dont have food, keep looking for it
                if (sandTimer.finished()) {
                    stuckTimer.start(300);
                    state = State::move;
                } else {
                    direction = rand() % 2;
                    if (direction == 0) {
                        direction = -1;
                    }
                    krembot.Base.drive(0, direction * turning_speed);
                }
            }
            break;
        }

        case State::block:{
            if (sandTimer.finished()) {
                state = State::move;
            }
            break;
        }

    }

}

CDegrees foraging_controller_2_controller::calculateDegreeHome() {
    Real y_distance = home_position.GetY() - pos.GetY();
    Real x_distance = home_position.GetX() - pos.GetX();
    CDegrees angle = CDegrees(atan2(y_distance,x_distance)*180/M_PI);
    return angle;
}

bool foraging_controller_2_controller::got_to_orientation(CDegrees degree) {
    if (((degreeX - degree).UnsignedNormalize().GetValue() > 0.50) &&
        ((degreeX - degree).UnsignedNormalize().GetValue() < 359.50)) {
        return false;
    } else {
        return true;
    }
}

Colors foraging_controller_2_controller::convertStringToColor(std::string col) {
    if (col == "green"){
        return Green;
    } else if (col == "blue"){
        return Blue;
    }else if (col == "red"){
        return Red;
    } else return None;
}

