%%Constantes
pM=36.5951;
KM=1.6665e+03;
Kp01=pM^2/(4*0.1^2*KM);
Kp03=pM^2/(4*0.3^2*KM);
Kp07=pM^2/(4*0.7^2*KM);
Kp1=pM^2/(4*1^2*KM);
reductora=75;
PulsosVuelta=3591.84;

%%Definición teórica de la función de transferencia
numbase=[KM];
denombase=[1 pM 0];
Gabierto=tf(numbase,denombase);
Gabiertored=Gabierto/75;
H01=feedback(Gabiertored*Kp01,1);
H03=feedback(Gabiertored*Kp03,1);
H07=feedback(Gabiertored*Kp07,1);
H1=feedback(Gabiertored*Kp1,1);

% Caso teórico
subplot(1,2,1); 
step(H1, H07, H03, H01);
xlabel('t');
ylabel('Amplitud');
title('Valores teóricos');
legend('con \xi,=1','con \xi=0,7', 'con \xi=0,3','con \xi=0,1', 'Location', 'SouthEast');

% Con real
it1=xlsread("Kp1","A1:A1201");
val1=xlsread("Kp1","B1:B1201");
it07=xlsread("Kp07","A1:A1201");
val07=xlsread("Kp07","B1:B1201");
it03=xlsread("Kp03","A1:A1201");
val03=xlsread("Kp03","B1:B1201");
it01=xlsread("Kp01","A1:A1201");
val01=xlsread("Kp01","B1:B1201");

subplot(1,2,2); 
plot(it1,val1/PulsosVuelta*2*pi);
hold on
plot(it07,val07/PulsosVuelta*2*pi);
plot(it03,val03/PulsosVuelta*2*pi);
plot(it01,val01/PulsosVuelta*2*pi);
plot( [0 1201], [pi pi] )
hold off
xlim([0 1200])
xlabel('t (ms)');
ylabel('Posición (rad)');
title('Valores reales');
legend('con \xi,=1','con \xi=0,7', 'con \xi=0,3','con \xi=0,1','\pi', 'Location', 'SouthEast');

 