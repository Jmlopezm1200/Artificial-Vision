close all;
clc;

% Configurar el puerto serial
puerto_serial = serialport("COM3", 9600);
configureTerminator(puerto_serial, "LF");

contador_muestras = 1;

figure; % Crear una nueva figura para plotear los datos
hold on;
plot(100, 0, 'or'); % Puntos de referencia (opcional)
title('Datos del Potenciómetro');
xlabel('X');
ylabel('Y');

% Leer y plotear datos
while contador_muestras <= 600
    if puerto_serial.NumBytesAvailable > 0
        % Leer una línea del puerto serial
        valor_potenciometro = readline(puerto_serial);

        % Dividir la cadena recibida por la coma
        C = strsplit(valor_potenciometro, ',');

        if length(C) == 2
            % Convertir los valores de cadena a número
            x = str2double(C{1});
            y = str2double(C{2});

            % Verificar si la conversión fue exitosa
            if ~isnan(x) && ~isnan(y)
                % Incrementar el contador de muestras
                contador_muestras = contador_muestras + 1;

                % Plotear los datos
                plot(x, y, '*');
                pause(0.01); % Pausa para actualizar la gráfica
            else
                disp('Datos inválidos recibidos');
            end
        else
            disp('Formato de datos incorrecto recibido');
        end
    else
        pause(0.01); % Pausa breve si no hay datos disponibles
    end
end

% Cerrar la conexión serial
clear puerto_serial;
