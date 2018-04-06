% This function computes matrix of fft waterfall values
% Signal - input 1-d signal
% dT_s   - discretization period in second
% Window_s - width of the fft window in second
% Overlay_s - width of the overlay between two consequent windows in seconds
function [vF,vT,Waterfall] = fftwaterfall(Signal, dT_s, Window_s, Overlay_s) 
  
  assert(Window_s > Overlay_s); 
  
  W = floor(Window_s / dT_s);  % counts per one window
  hsW = floor(W/2);
  O = floor(Overlay_s / dT_s); % counts in overlay
  
  Num_of_windows = floor((length(Signal) - W) / (W - O));  
  
  Waterfall = zeros(Num_of_windows,hsW);
  
  TempS = Signal(1:W);
  for i=1:Num_of_windows      
      if i > 1
        TempS = Signal((i-1)*(W-O)+1:(i-1)*(W-O)+W);
      end
      TempF = abs(fft(TempS));
      for j=1:hsW
        Waterfall(i,j) = TempF(j);
      end
  end 
  
  vT = zeros(Num_of_windows,1);
  for j=1:length(vT)
    vT(j) = j*(Window_s - Overlay_s);  
  end
   
  stepF_Hz = (1.0 / dT_s) / W;
  vF = zeros(hsW,1);
  for j=1:length(vF);        
    vF(j) = j*stepF_Hz;
  end 
end