clc;
clear all;
close all;

MATH_CLC_Q1=(2);
MATH_CLC_Q2=(4);
MATH_CLC_Q3=(8);
MATH_CLC_Q4=(16);
MATH_CLC_Q5=(32);
MATH_CLC_Q6=(64);
MATH_CLC_Q7=(128);
MATH_CLC_Q8=(256);
MATH_CLC_Q9=(512);
MATH_CLC_Q10=(1024);
MATH_CLC_Q11=(2048);
MATH_CLC_Q12=(4096);
MATH_CLC_Q13=(8192);
MATH_CLC_Q14=(16384);
MATH_CLC_Q15=(32768);
%%
x = linspace(-2*pi,2*pi);
y1 = sin(x);
y2 = cos(x);

figure 
plot(x,y1,x,y2);
grid;

%%
y_data= zeros(1,100);%创建一个一维数组
x_data= zeros(1,100);%创建一个一维数组
k1 = 1;
k2 = 0;
output = -500;
limt_up = 0.5;
limt_down = -0.3;
y_scope = 0;

for Step = 0:1:100
%------------------------- 
%     y_scope = Step*sin(Step);
    err = output - Step;
   
    if(err>limt_up)
    err = limt_up;
    end
    if(err<limt_down)
    err = limt_down;
    end
   
    y_scope = y_scope+err;
   
 %------------------------- 
    k2 = k2+1;
    x_data(1,k2)=Step;
    y_data(1,k2)=y_scope;
end     

%%
figure 
plot(x_data,y_data)
grid;


