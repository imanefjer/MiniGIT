#ifndef MINIVERSIONCONTROL_H
#define MINIVERSIONCONTROL_H

#include <filesystem>
#include <chrono>
#include <string>
#include <mutex>

namespace fs = std::filesystem;
using namespace std::chrono;

class MiniVersionControl
{
public:
    MiniVersionControl();

    void init();
    void add(const std::string& path);
    void commit(const std::string& message);
    void revert(const std::string& commitFolder);
    std::vector<std::string> listFilesAndFolders();
    void revertFile(const fs::path& source, const fs::path& destination);
    void addFile(const fs::path& source, const fs::path& destination);
    std::vector<std::string> listStagingArea();


    std::vector<std::string> listVersions();


    void revertDirectory(const fs::path& sourceDir, const fs::path& destinationDir);


    void addDirectory(const fs::path& source, const fs::path& destination);

    void deleteFromStaging(const std::string& name);

    void enhancedAddDirectoy(const fs::path& source, const fs::path& destination);



private:
    // Your class members go here
    std::mutex mutex_; // Mutex for synchronization

};

#endif // MINIVERSIONCONTROL_H
