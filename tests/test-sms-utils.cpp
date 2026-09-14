#include <sms-utils.h>
#include <string>
#include <iostream>

int main(int argc, char *argv[]) {
    if(argc != 2) {
        std::cout << "Error. No unit argument!";
        return -1;
    }
    std::string unit(argv[1]);

    SmsModule sms_module;

    if(unit == "ValidatePhoneNumber") {
        if(sms_module.ValidatePhoneNumber("hello world")) return 1;
        if(sms_module.ValidatePhoneNumber("09133333333")) return 1;
        if(sms_module.ValidatePhoneNumber("922920920")) return 1;
        if(sms_module.ValidatePhoneNumber("+989131231")) return 1;
        if(sms_module.ValidatePhoneNumber("+989131231212")) return 1;
        if(!sms_module.ValidatePhoneNumber("+989027732097")) return 1;
    }

    return 0;
}