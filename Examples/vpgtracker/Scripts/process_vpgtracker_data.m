% This script should be used along with the *.csv files that have been produced
% by vpgtracker application to make visualization of the data

% Alex_A._Taranov aka pi-null-mezon 04.04.2018
function [] = process_vpgtracker_data(inputfilename)
  
  disp('-----------------------');
  disp('File parsing has been started. Please wait...');
  
  inputfile = fopen(inputfilename);
  
  % First we need to check file version
  line = fgetl(inputfile); % first line should contain vpgtracker version, we must check that is greater than 1.0.0.0
  release_version = str2num(line(length(line))); 
  revision_version = str2num(line(length(line) - 2)); 
  minor_version = str2num(line(length(line) - 4));  
  major_version = str2num(line(length(line) - 6));
  disp(['vpgtracker file version: ' num2str(major_version) '.' num2str(minor_version) '.' num2str(revision_version) '.' num2str(release_version)]); 
  if major_version < 1
    disp('Unsupported version of the vpgtracker file, must be greater than 1.0.1.0');  
    fclose(inputfile);
    return;    
  else
    if revision_version < 1
    disp('Unsupported version of the vpgtracker file, must be greater than 1.0.1.0'); 
    fclose(inputfile);
    return; 
  end

  % Lets find out research date 
  line = fgetl(inputfile);
  date_str = line(23:32);
  time_str = line(34:length(line));
  disp(['Date: ' date_str '; Time: ' time_str]);
  % And we are ready to read discretization period of the data
  line = fgetl(inputfile);
  dT_s = str2num(line(27:findstr(line,'[ms]') - 1)) / 1000.0;
  disp(['Discretization period: ' num2str(dT_s*1000.0) ' ms']);
  fclose(inputfile);
  
  % Now we can read measurements
  M = csvread(inputfilename,6,0); % It is intersting to note that csvread(...) use 0-start index scheme
  
  vT = zeros(size(M,1),1);
  for i = 1:length(vT)
    vT(i) = i*dT_s;
  end

  % Lets plot out vpg signals for the selected zones
  figure
  subplot(2,1,1);
  plot(vT,M(:,19),'r');
  title('VPG signal for selection zone one');
  axis([0,vT(end), -3.0, 3.0]);
  grid on
  
  subplot(2,1,2);
  plot(vT,M(:,20),'b');
  title('VPG signal for selection zone two');
  axis([0,vT(end), -3.0, 3.0]);
  grid on
    
  % Lets plot spectrum waterfall
  Duration_s = 30.0;
  Overlay_s  = 29.0; 
  figure 
  [wfF,wfT,wfE] = fftwaterfall(M(:,19),dT_s,Duration_s,Overlay_s);
  image(wfF,wfT,wfE);
  %waterfall(wfF,wfT,wfE);
  title('Fourier spectrum waterfall for selection zone one');
  xlabel('Frequency, Hz');
  ylabel('Time, s');
  colorbar;
  
  figure  
  [wfF,wfT,wfE] = fftwaterfall(M(:,20),dT_s,Duration_s,Overlay_s);
  image(wfF,wfT,wfE);
  %waterfall(wfF,wfT,wfE);
  title('Fourier spectrum waterfall for selection zone two');
  xlabel('Frequency, Hz');
  ylabel('Time, s');
  colorbar;
  
  disp('-----------------------');  
end

