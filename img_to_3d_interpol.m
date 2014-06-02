% interpolation of image coordinates to 3D using img01.jpg

x_pxl = [24, 48, 72, 96, 122, 147, 174, 200, 228, 254, ...
    284, 314, 344, 368, 400, 429, 466, 500, 536, 572, ...
    609, 646, 687, 724, 771, 813, 855, 902, 951];           % [pxl]
y_mm = [10:10:290];                                         % [mm] distance from top of image

P = polyfit(x_pxl, y_mm, 2)

figure;
plot(x_pxl, y_mm, 'b', x_pxl, polyval(P, x_pxl), 'r');
