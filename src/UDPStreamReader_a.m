classdef UDPStreamReader_a < matlab.System
    properties
        LocalPort = 8888; % Port UDP on l'ESP32 envia les dades
    end
    
    properties(Access = private)
        UdpObj
        LastImg
    end
    
    methods(Access = protected)
        function setupImpl(obj)
            coder.extrinsic('udpport', 'udpportfind');
            
            % 1. Neteja preventiva real fent servir udpportfind
            try
                oldPorts = udpportfind("LocalPort", obj.LocalPort);
                delete(oldPorts);
            catch
            end
            
            % 2. Obertura del socket UDP
            obj.UdpObj = udpport("byte", "LocalPort", obj.LocalPort, "Timeout", 0.1);
            obj.LastImg = zeros(240, 320, 3, 'uint8');
        end
        
        function img = stepImpl(obj)
            coder.extrinsic('read', 'imresize', 'decodeJpegInMemory');
            
            img = obj.LastImg;
            
            if ~isempty(obj.UdpObj) && isvalid(obj.UdpObj) && obj.UdpObj.NumBytesAvailable > 0
                % Llegim tots els bytes acumulats al port UDP
                rawBytes = read(obj.UdpObj, obj.UdpObj.NumBytesAvailable, "uint8");
                rawBytes = uint8(rawBytes);
                
                % Busquem el marcador d'inici (SOI: 0xFF 0xD8 -> 255 216) 
                % i de final (EOI: 0xFF 0xD9 -> 255 217) del fotograma JPEG
                soi = find(rawBytes(1:end-1) == 255 & rawBytes(2:end) == 216, 1, 'first');
                eoi = find(rawBytes(1:end-1) == 255 & rawBytes(2:end) == 217, 1, 'last');
                
                if ~isempty(soi) && ~isempty(eoi) && eoi > soi
                    jpegBytes = rawBytes(soi : eoi + 1);
                    decoded = decodeJpegInMemory(jpegBytes);
                    
                    if ~isempty(decoded)
                        if size(decoded, 1) ~= 240 || size(decoded, 2) ~= 320
                            img = imresize(decoded, [240, 320]);
                        else
                            img = decoded;
                        end
                        obj.LastImg = img;
                    end
                end
            end
        end
        
        function releaseImpl(obj)
            % CLAU: delete() tanca el socket de xarxa al SO, clear() no ho cap.
            if ~isempty(obj.UdpObj) && isvalid(obj.UdpObj)
                delete(obj.UdpObj);
                obj.UdpObj = [];
            end
        end
        
        % --- Mètodes de propagació per a Simulink ---
        function out = getOutputSizeImpl(~)
            out = [240 320 3];
        end
        
        function out = getOutputDataTypeImpl(~)
            out = 'uint8';
        end
        
        function out = isOutputComplexImpl(~)
            out = false;
        end
        
        function out = isOutputFixedSizeImpl(~)
            out = true;
        end
    end
end

function img = decodeJpegInMemory(jpegBytes)
    % Descodifica el flux de bytes JPEG en memòria fent servir les llibreries Java natives de MATLAB
    img = [];
    try
        stream = java.io.ByteArrayInputStream(jpegBytes);
        jImg = javax.imageio.ImageIO.read(stream);
        
        if ~isempty(jImg)
            w = jImg.getWidth();
            h = jImg.getHeight();
            pixels = jImg.getRGB(0, 0, w, h, [], 0, w);
            
            % Extracció dels canals RGB des del format ARGB de Java
            r = uint8(bitand(bitshift(pixels, -16), 255));
            g = uint8(bitand(bitshift(pixels, -8), 255));
            b = uint8(bitand(pixels, 255));
            
            img = zeros(h, w, 3, 'uint8');
            img(:,:,1) = reshape(r, [w, h])';
            img(:,:,2) = reshape(g, [w, h])';
            img(:,:,3) = reshape(b, [w, h])';
        end
    catch
        img = [];
    end
end