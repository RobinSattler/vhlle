#include "vtk.h"
#include "hdo.h"
#include "fld.h"
#include "eos.h"

#include <stdexcept>
#include <iostream>
#include <sstream>
#include <vector>

void VtkOutput::write_header(std::ofstream &file, const Hydro h, const std::string &description){
  if(cartesian_){
    file << "# vtk DataFile Version 2.0\n"
      << description << "\n"
      << "ASCII\n"
      << "DATASET STRUCTURED_POINTS\n"
      << "DIMENSIONS " << h.getFluid()->getNX() << " " << h.getFluid()->getNY() << " " << (h.getFluid()->getNZ())*20 << "\n"
      << "SPACING " << h.getFluid()->getDx() << " " << h.getFluid()->getDy() << " " << h.getTau()*std::sinh(h.getFluid()->getDz()*(h.getFluid()->getNZ()/2))/(20*h.getFluid()->getNZ()/2) << "\n"
      << "ORIGIN " << xmin_ << " " << ymin_ << " " << zmin_ << "\n"
      << "POINT_DATA " << h.getFluid()->getNX()*h.getFluid()->getNY()*(h.getFluid()->getNZ())*20<< "\n";
  }else{
    file << "# vtk DataFile Version 2.0\n"
      << description << "\n"
      << "ASCII\n"
      << "DATASET STRUCTURED_POINTS\n"
      << "DIMENSIONS " << h.getFluid()->getNX() << " " << h.getFluid()->getNY() << " " << h.getFluid()->getNZ() << "\n"
      << "SPACING " << h.getFluid()->getDx() << " " << h.getFluid()->getDy() << " " << h.getFluid()->getDz() << "\n"
      << "ORIGIN " << xmin_ << " " << ymin_ << " " << zmin_ << "\n"
      << "POINT_DATA " << h.getFluid()->getNX()*h.getFluid()->getNY()*h.getFluid()->getNZ() << "\n";
  }
  
  return;
}

std::string VtkOutput::make_filename(const std::string &descr, int counter) {
  char suffix[24];
  snprintf(suffix, sizeof(suffix), "_taustep%05i.vtk",counter);
  return path_ + std::string("/") + descr + std::string(suffix);
}

void VtkOutput::write_vtk_scalar(std::ofstream &file, const Hydro h, std::string &quantity) {
  file << "SCALARS " << quantity << " double 1\n"
       << "LOOKUP_TABLE default\n";
  file << std::setprecision(3);
  file << std::fixed;
  int z_length;
  if(cartesian_){
    z_length=20*(h.getFluid()->getNZ());
  }else{
    z_length=h.getFluid()->getNZ();
  }
  
  for (int iz = 0; iz < z_length; iz++) {
    double poseta=iz;
    double factor=1;
    if(cartesian_){
      double total_length=h.getTau()*std::sinh(h.getFluid()->getDz()*(h.getFluid()->getNZ()/2));
      double pos;
      if(iz<z_length/2){
        pos=-total_length+total_length/(z_length/2)*iz;
        for (int ieta=0; ieta< h.getFluid()->getNZ()/2; ieta++){
        if(h.getTau()*std::sinh(h.getFluid()->getDz()*(ieta-h.getFluid()->getNZ()/2))>pos){
          factor=fabs((h.getTau()*std::sinh(h.getFluid()->getDz()*(ieta-h.getFluid()->getNZ()/2))-pos)/pos);
          poseta=ieta;
          break;
        }
      }
      }else{
        pos=total_length/(z_length/2)*(iz-z_length/2);
        for (int ieta=h.getFluid()->getNZ()/2; ieta<h.getFluid()->getNZ() ; ieta++){
        if(h.getTau()*std::sinh(h.getFluid()->getDz()*(ieta-h.getFluid()->getNZ()/2))>pos){
          factor=fabs((h.getTau()*std::sinh(h.getFluid()->getDz()*(ieta-h.getFluid()->getNZ()/2))-pos)/pos);
          poseta=ieta;
          break;
        }
      }
      }
      if(pos==0){
        factor=1;
      }
     }
   
    for (int iy = 0; iy < h.getFluid()->getNY(); iy++) {
      for (int ix = 0; ix < h.getFluid()->getNX(); ix++) {
         double e,p,nb,nq,ns,vx,vy,vz, T, mub, muq, mus;
         double e2,p2,nb2,nq2,ns2,vx2,vy2,vz2, T2, mub2, muq2, mus2;
         Cell* cell=h.getFluid()->getCell(ix,iy,poseta);
         Cell* cell2;
         if(poseta>0){
           cell2=h.getFluid()->getCell(ix,iy,poseta-1);
         }else{
           cell2=cell;
         }
         #ifdef CARTESIAN
         cell->getPrimVar(eos_, 1.0, e, p, nb, nq, ns, vx, vy, vz);
         cell2->getPrimVar(eos_, 1.0, e2, p2, nb2, nq2, ns2, vx2, vy2, vz2);
         #else
         cell->getPrimVar(eos_, h.getTau(), e, p, nb, nq, ns, vx, vy, vz);
         cell2->getPrimVar(eos_, h.getTau(), e2, p2, nb2, nq2, ns2, vx2, vy2, vz2);
         #endif
         double q=0;
         if (quantity=="eps"){
           q=factor*e+(factor-1)*e2;
         } else if (quantity == "p"){
           q=factor*p+(factor-1)*p2;
         } else if (quantity =="nb"){
           q=factor*nb+(factor-1)*nb2;
         } else if (quantity =="nq"){
           q=factor*nq+(factor-1)*nq2;
         } else if(quantity == "ns"){
           q=factor*ns+(factor-1)*ns2;
         } else if(quantity == "T"){
           eos_->eos(e, nb, nq, ns, T, mub, muq, mus, p);
           eos_->eos(e2, nb2, nq2, ns2, T2, mub2, muq2, mus2, p2);
           q=factor*T+(factor-1)*T2;
          } else if(quantity == "mub"){
           eos_->eos(e, nb, nq, ns, T, mub, muq, mus, p);
           eos_->eos(e2, nb2, nq2, ns2, T2, mub2, muq2, mus2, p2);
           q=factor*mub+(factor-1)*mub2;
          } else if(quantity == "muq"){
           eos_->eos(e, nb, nq, ns, T, mub, muq, mus, p);
           eos_->eos(e2, nb2, nq2, ns2, T2, mub2, muq2, mus2, p2);
           q=factor*muq+(factor-1)*muq2;
          } else if(quantity == "mus"){
           eos_->eos(e, nb, nq, ns, T, mub, muq, mus, p);
           eos_->eos(e2, nb2, nq2, ns2, T2, mub2, muq2, mus2, p2);
           q=factor*mus+(factor-1)*mus2;
         }else{
          std::cout<<"No quantity for VTK output specified"<<std::endl;
          return;
         }
         file << q << " ";
      }
      file << "\n"; 
    }
  }
}

std::vector<std::string> split (const std::string &s, char delim) {
  std::vector<std::string> result;
  std::stringstream ss (s);
  std::string item;

  while (getline (ss, item, delim)) {
      result.push_back (item);
  }

  return result;
}

void VtkOutput::write(const Hydro h, std::string &quantity){
  std::vector<std::string> quantity_list=split(quantity,',');
  vtk_output_counter_++;
  for (auto q : quantity_list){
    std::string t=q;
    std::ofstream file;

    file.open(make_filename(t, vtk_output_counter_), std::ios::out);
    write_header(file, h, t);
    write_vtk_scalar(file, h, q);
  }
  
  return;
}
