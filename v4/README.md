

LPM012M134B code: (选型放弃这款，颜色不好看)
* https://down.cnwans.com/archives/509
* https://github.com/andelf/rp-embassy-playground/blob/master/src/bin/jdi-1in2-round.rs


选择带背光的 LS012B7DD06
* 并口 RGB222

# 关于功耗和常亮
* 需要 esp32-s3 深睡眠之前，调用 gpio_hold_en 或者开启全部 gpio_deep_sleep_hold_en()
* 上面两者实测没有功耗差别
* pwm ledc pin 没有被 hold，背光会熄灭
