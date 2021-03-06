#include "Walker.h"

void Walker::init()
{
  // init_f("Walker");
}

void Walker::terminate()
{
  controller.printDisplay(1, "Stopped.");
  controller.leftWheel.stop();
  controller.rightWheel.stop();
}

void Walker::reset()
{
  controller.leftWheel.reset();
  controller.rightWheel.reset();
}

void Walker::stop()
{
  run(0, 0);

  controller.printDisplay(1, "stopping...");
  controller.leftWheel.reset();
  controller.rightWheel.reset();
}

void Walker::run(std::int8_t pwm, std::int8_t turn)
{
  controller.printDisplay(1, "running...");
  setBrakeMotor(false);
  /* left = p-t, right = p+t -> 右 */
  /* left = p+t, right = p-t -> 左 */
  // pwmは int型の -100 ~ 100
  controller.leftWheel.setPWM(pwm - turn);
  controller.rightWheel.setPWM(pwm + turn);
}

void Walker::setBrakeMotor(bool brake)
{
  // 0でフロート
  // 1でブレーク
  controller.leftWheel.setBrake(brake);
  controller.rightWheel.setBrake(brake);
}

std::int32_t Walker::get_count_L()
{
  return controller.leftWheel.getCount();
}

std::int32_t Walker::get_count_R()
{
  return controller.rightWheel.getCount();
}

int Walker::edgeChange()
{
  if(leftRight == 1) {
    run(10, 5);
    controller.clock.sleep(10);
    leftRight = -1;
  } else {
    run(10, 5);
    controller.clock.sleep(10);
    leftRight = 1;
  }

  return leftRight;
}

/*
 * 車輪の回転角分だけ進む
 */
void Walker::moveAngle(std::int8_t pwm, int angle)
{
  controller.leftWheel.reset();
  controller.rightWheel.reset();

  controller.leftWheel.setPWM(pwm);
  controller.rightWheel.setPWM(pwm);

  while(1) {
    if(controller.leftWheel.getCount() >= angle && controller.rightWheel.getCount() >= angle) break;
    controller.clock.sleep(4);
  }

  controller.leftWheel.reset();
  controller.rightWheel.reset();
}

/*
 * 主機能:45度単位で回転
 * rotation = 1 -> 反時計回り, rotation = -1 -> 時計回り
 *
 * beta機能:5度単位で回転
 * 精度はあまりよろしくない
 */
void Walker::angleChange(int angle, int rotation)
{
  int32_t defaultAngleL;
  int8_t dAngle = 75;  // 45度におけるモーター回転数（床材によって変わる？）

  if(rotation >= 0) {
    if(leftRight == 1) {
      rotation = 1;
    } else {
      rotation = -1;
    }
  } else {
    if(leftRight == 1) {
      rotation = -1;
    } else {
      rotation = 1;
    }
  }

  /*
   * 本来は45度単位だから、angleは45で割る
   * ベータ機能として5度単位でも曲がれるようにしている
   * そのため、もしangleが5度単位である場合はdAngleを9分割する
   */
  if(angle % 5 == 0 && angle % 45 != 0) {
    dAngle = 8;
    angle /= 5;
  } else {
    angle -= angle % 45;
    angle /= 45;
  }

  defaultAngleL = controller.leftWheel.getCount();

  while(1) {
    run(0, 10 * rotation);
    if(rotation >= 0) {
      if(controller.leftWheel.getCount() - defaultAngleL < -dAngle * angle * rotation
         || controller.leftWheel.getCount() - defaultAngleL > dAngle * angle * rotation) {
        break;
      }
    } else {
      if(controller.leftWheel.getCount() - defaultAngleL > -dAngle * angle * rotation
         || controller.leftWheel.getCount() - defaultAngleL < dAngle * angle * rotation) {
        break;
      }
    }
    controller.clock.sleep(4);
  }
  stop();
}
