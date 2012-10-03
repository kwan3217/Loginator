function doJac
  %Generate the physics functions and jacobians
  syms ew ex ey ez wx wy wz rx ry rz vx vy vz aix aiy aiz gz m n Bix Biy Biz real;
  
  e=[ex;ey;ez];
  w=[wx;wy;wz];
  ai=[aix;aiy;aiz];
  r=[rx;ry;rz];
  v=[vx;vy;vz];
  x=[ew;e;w;r;v]
  etilde=[ 0, -ez, ey
           ez, 0, -ex
          -ey, ex, 0];
  %This is the equivalent matrix to the quaternion. The quaternion is used
  %to transform from inertial to body as Ab=e~*Ai*e
  E=2*(e*e')+(ew^2-e'*e)*eye(3)-2*ew*etilde;
  ab=E'*(ai+[0;0;gz]);
  Bi=[Bix;Biy;Biz];
  EBi=E*Bi;
  Fe=[quatmultiply([ew,e'],[0,w'])'/2]
  Fw=[0;0;0];
  Fr=[vx;vy;vz];
  Fv=[ai];
  F=[Fe;Fw;Fr;Fv]
  gw=w;
  gb=EBi;
  gr=r;
  gv=v;
  ga=ab;
  g=[gw;gb;ga]
  Phi=jacobian(F,x)
  H=jacobian(g,x)
  matlabFunction(F,  'file','F_IMU.m')
  matlabFunction(g,  'file','g_IMU.cf')
  matlabFunction(Phi,'file','Phi_IMU.m')
  matlabFunction(H,  'file','H_IMU.cf')

end