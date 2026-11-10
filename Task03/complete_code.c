#include <stdio.h>
enum kind { TEMPERATURE, HUMIDITY, STATUS };
struct reading {
    enum kind type;                 /* tag: tells which union member is valid */
    union {
        float celsius;
        int   percent;
        char  status[8];
    } value;
};
void show(const struct reading *r)
{
    // TODO 1: Use a switch on r->type to print only the valid union member: "Temp 31.5
    //   C", "Humidity 68 %" or "Status OK"
    switch(r->type){
        case TEMPERATURE:printf("Temp %.1f C\n",r->value.celsius);break;
        case HUMIDITY:printf("Humidity %d %%\n",r->value.percent);break;
        case STATUS:printf("Status %.7s\n",r->value.status);break;
        default:puts("Invalid packet tag");break;
    }
}
int main(void)
{
    struct reading log[3] = {
        {TEMPERATURE, {.celsius = 31.5f}},
        {HUMIDITY,    {.percent = 68}},
        {STATUS,      {.status = "OK"}}
    };
    printf("sizeof union = %zu, sizeof struct reading = %zu\n",
           sizeof log[0].value, sizeof log[0]);
    for (int i = 0; i < 3; i++) show(&log[i]);
    return 0;
}
