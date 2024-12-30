#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

class Cell;
class EoS;
class Fluid;
class Hydro;

class VtkOutput {
  private:
    std::string path_;
    EoS* eos_;
    double xmin_, ymin_, etamin_;
    bool cartesian_;
    int vtk_output_counter_ = 0;  // Number of vtk output in current event
    /*
     * The following vector contains all currently supported VTK quantities.
     * If multiple quantities are desired, the delimiter in the config file has
     * to be a comma without any whitespaces in between, for example:
     * `VTK_output_valus eps,mub,nq,T,v,pi`
     * For vector quantities only the corresponding three vector will be written
     * to the output file.
     */
    const std::map<std::string, std::string> valid_quantities_ = {
      {"eps", "scalar"},        // energy density
      {"mub", "scalar"},        // baryon chemical potential
      {"muq", "scalar"},        // electric chemical potential
      {"mus", "scalar"},        // strangeness chemical potential
      {"nb", "scalar"},         // baryon density
      {"nq", "scalar"},         // charge density
      {"ns", "scalar"},         // strangeness density
      {"p", "scalar"},          // pressure
      {"Pi", "scalar"},         // bulk pressure
      {"pi", "tensor"},         // shear stress tensor
      {"T", "scalar"},          // temperature
      {"v", "vector"}           // velocity
    };

  public:
    /**
     * Create a new VTK output.
     *
     * \param path Path to the output file.
     * \param eos The equation of state.
     * \param xmin x coordinate of the first cell (center of fluid cell).
     * \param ymin y coordinate of the first cell (center of fluid cell).
     * \param etamin eta coordinate of the first cell (center of fluid cell).
     * \param cartesian Bool that states if Cartesian output is enabled.
     */
    VtkOutput(std::string path, EoS* eos, double xmin, double ymin,
              double etamin, bool cartesian=false):
                path_(path),
                eos_(eos),
                xmin_(xmin),
                ymin_(ymin),
                etamin_(etamin),
                cartesian_(cartesian)
              {}

    void write(const Hydro h, std::string &quantity);
    void write_header(std::ofstream &file, const Hydro h,
                      const std::string &description);
    void write_vtk_scalar(std::ofstream &file, const Hydro h,
                          std::string &quantity);
    void write_vtk_vector(std::ofstream &file, const Hydro h,
                          std::string &quantity);
    void write_vtk_tensor(std::ofstream &file, const Hydro h,
                          std::string &quantity);
    bool is_quantity_implemented(std::string &quantity);
    std::vector<double> smearing_factor_and_poseta(const Hydro h, const int iz,
                                    const int z_length);
    std::string make_filename (const std::string &descr, int counter);
};
