function [angle, max_H, curve_visible_distance, point1, point2] = hough_line_detection(gray)


imblur = imgaussfilt(gray,1);
prewittImg = edge(imblur, 'prewitt');             % Canny (filtratge pre)

% GRAY IMAGE EDGE CUT
prewittImg(1:10,:) = 0; % fila, columna
prewittImg(:,1:10) = 0;
prewittImg(:,end - 10:end) = 0;
prewittImg(end - 10:end,:) = 0;

[H, T, R] = hough(prewittImg);                  % H emmagatzema el compte de vots de la im binària de vores. 
max_H = max(H,[],"all");

thresh = max_H;
P = houghpeaks(H, 10, 'Threshold', thresh);     % Matriu conté coordenades
                                                % dels peaks, no el valor d'aquests
lines_hough = houghlines(prewittImg, T, R, P,"FillGap",5,"MinLength",50); % lines_hough = houghlines(img, theta, rho, peaks, línies amb distància
if isempty(lines_hough)
    angle = 0;
    curve_visible_distance = 0;
    disp('no lines detected');
    point1 = [0, 0];
    point2 = [0, 10];
else
    angle = lines_hough(1).theta;
    point1 = lines_hough(1).point1;
    point2 = lines_hough(1).point2;
    curve_visible_distance = lines_hough(1).point1(1,2); % !!!! dist visible camera i cal revisar resultat
    disp(['angle = ', num2str(angle)]);
    disp(['distance = ', num2str(curve_visible_distance)]);
end
end
