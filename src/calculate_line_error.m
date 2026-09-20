function [error, x_centroid] = calculate_line_error(bw)
                                        % Image resolution from Video Viewer: 480 rows x 640 columns
    [height, width] = size(bw);
    x_center = width / 2; % Image center is x = 320

                                        % Focus on a Region of Interest (ROI) at the bottom 25% of the frame
    roi_start = round(height * 0.75);
    roi = bw(roi_start:end, :);         % bw(files, columnes) == bw(from roi_start to end (Yaxis), from begining to end (Xaxis)
    
    [~, cols] = find(roi > 0);
    % cols = [1; 2; 2] (exemple diferent)
    
    if ~isempty(cols) % = if 1 then
        x_centroid = mean(cols);        % Calculate average X-position of line
        error = x_centroid - x_center;  % Lateral distance from center
    else                                % = else (if 0 then)
        x_centroid = x_center;          % Fallback if line is lost
        error = 0;
    end
end