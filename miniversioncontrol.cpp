#include <miniversioncontrol.h>

#include <iostream>
#include <sstream>
#include <ctime>
#include <string>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <mutex>
#include <vector>
#include <cstring>
#include <future>

namespace fs = std::filesystem;
using namespace std::chrono;

/**
 * @brief MiniVersionControl class constructor.
 */
MiniVersionControl::MiniVersionControl() {
    // Constructor;
}

namespace fs = std::filesystem;

/**
 * @brief Logger class for logging messages to a file.
 */
class Logger {
public:
    /**
     * @brief Logs a message to a file with a timestamp.
     * @param message The message to be logged.
     */
    static void log(const std::string& message) {
        std::ofstream logFile("log.txt", std::ios::app);
        if (logFile.is_open()) {
            auto timestamp = system_clock::to_time_t(system_clock::now());
            std::tm timeinfo = *std::localtime(&timestamp);
            logFile << "[" << std::put_time(&timeinfo, "%d/%m/%Y %H:%M") << "]" << message << "\n";
            logFile.close();
        }
    }
};

/**
 * @brief Computes a checksum for a given string using the FNV-1a hash algorithm.
 * @param str The string for which the checksum is calculated.
 * @return uint32_t - The computed checksum.
 */
uint32_t computeChecksum(const std::string& str) {
    const uint32_t prime = 16777619;
    uint32_t hash = 2166136261U;

    for (char ch : str) {
        hash ^= static_cast<uint32_t>(ch);
        hash *= prime;
    }

    return hash;
}

/**
 * @brief Initializes the version control system by creating necessary directories.
 */
void MiniVersionControl::init() {
    try {
        fs::create_directory(".git");
        fs::create_directory(".git/commits");
        fs::create_directory(".git/staging");
    } catch (const std::exception& e) {
        Logger::log("Error during initialization: " + std::string(e.what()));
        throw;
    }
}

/**
 * @brief Adds a file or directory to the staging area.
 * @param path The path of the file or directory to be added.
 */
void MiniVersionControl::add(const std::string& path) {
    try {
        const auto copyOptions = fs::copy_options::overwrite_existing | fs::copy_options::recursive;
        // Add a specific file or directory to the staging area
        std::string destinationPath = ".git/staging/" + fs::path(path).filename().string();
        if (fs::exists(destinationPath)) {
            try {
                fs::remove_all(destinationPath);
            } catch (const std::exception& e) {
                Logger::log("Error removing existing file or directory: " + std::string(e.what()));
                throw;
            }
        }

        if (fs::is_regular_file(path)) {
            addFile(path, destinationPath);
        } else if (fs::is_directory(path)) {
            addDirectory(path, destinationPath);
        } else {
            Logger::log("Error: Invalid file or directory path.");
        }
    } catch (const std::exception& e) {
        Logger::log("Error adding file or directory: " + std::string(e.what()));
        throw;
    }
}

/**
 * @brief Recursively adds files from a source directory to the staging area.
 * @param source The source directory.
 * @param destination The destination directory in the staging area.
 */
void MiniVersionControl::addDirectory(const fs::path& source, const fs::path& destination) {
    // Process directories recursively
    fs::create_directory(destination);
    for (const auto& entry : fs::recursive_directory_iterator(source)) {
        fs::path nestedDestination = destination / entry.path().lexically_relative(source);

        if (entry.is_regular_file()) {
            MiniVersionControl::addFile(entry.path(), nestedDestination);
        } else if (entry.is_directory()) {
            addDirectory(entry.path(), nestedDestination);
        }
    }
}

/**
 * @brief Adds a file to the staging area.
 * @param source The source file.
 * @param destination The destination file in the staging area.
 */
void MiniVersionControl::addFile(const fs::path& source, const fs::path& destination) {
    try {

        std::lock_guard<std::mutex> lock(MiniVersionControl::mutex_);

        std::ifstream inputFile(source, std::ios::binary);
        if (!inputFile.is_open()) {
            std::string errorMessage = "Error opening source file: " + source.string();
            Logger::log(errorMessage);
            throw std::runtime_error(errorMessage);
        }
        std::ostringstream contentBuffer;
        // Add "1234" at the beginning
        contentBuffer << "1234";
        contentBuffer << inputFile.rdbuf();
        inputFile.close();

        // Calculate checksum
        uint32_t checksum = computeChecksum(contentBuffer.str());
        // Convert checksum to a 4-byte array
        char checksumBytes[4];
        std::memcpy(checksumBytes, &checksum, sizeof(checksum));
        // Append checksum at the end of the content
        contentBuffer.write(checksumBytes, sizeof(checksum));
        // Convert fs::path to std::string for the destination
        std::string destinationStr = destination.string();
        // Write content to the destination file
        std::ofstream outputFile(destinationStr, std::ios::binary);
        if (!outputFile.is_open()) {
            std::string errorMessage = "Error opening destination file: " + destinationStr;
            Logger::log(errorMessage);
            throw std::runtime_error(errorMessage);
        }
        // Write content to the destination file
        outputFile << contentBuffer.str();
        outputFile.close();
    } catch (const std::exception& e) {
        Logger::log("Error adding file: " + std::string(e.what()));
        throw;
    }
}

/**
 * @brief Commits the changes in the staging area to a new version.
 * @param message The commit message.
 */
void MiniVersionControl::commit(const std::string& message) {
    try{
        if (fs::is_empty(".git/staging")) {
            Logger::log("Error: Staging area is empty. Nothing to commit.");
            return;
        }

        // Create a unique folder for each commit using timestamp
        auto timestamp = duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
        std::string commitFolder = ".git/commits/" + std::to_string(timestamp);
        fs::create_directory(commitFolder);

        // Move files from staging area to the commit folder
        for (const auto& entry : fs::directory_iterator(".git/staging")) {
            if (fs::is_directory(entry)) {
                // Copy directories recursively
                // TODO: Locks
                fs::copy(entry, commitFolder / entry.path().filename(), fs::copy_options::recursive);
            } else {
                // Copy files as before
                // TODO: Locks
                fs::copy(entry, commitFolder / entry.path().filename());
            }
        }

        std::ofstream commitFile(commitFolder + "/commit_info.txt");
        commitFile << "Author: Fjer\n";
        commitFile << "Date: " << ctime(&timestamp);
        commitFile << "Message: " << message << "\n";

        // Clear the staging area
        fs::remove_all(".git/staging");
        fs::create_directory(".git/staging");
    }
    catch (const std::exception& e) {
        Logger::log("Error committing: " + std::string(e.what()));
        throw;
    }
}

/**
 * @brief Reverts the files and directories in a specified commit to the previous state.
 * @param commitFolder The folder containing the commit to be reverted.
 */
void MiniVersionControl::revert(const std::string& commitFolder) {
    try {
        for (const auto& entry : fs::directory_iterator(commitFolder)) {
            if (entry.is_regular_file() || entry.is_directory()) {
                if (entry.path().filename() == "commit_info.txt") {
                    continue;
                }

                fs::path destinationPath = fs::current_path() / entry.path().filename();

                if (fs::is_directory(entry)) {
                    // Revert directories recursively
                    revertDirectory(entry.path(), destinationPath);
                } else {
                    // Revert individual files
                    revertFile(entry.path(), destinationPath);
                }
            }
        }
    }
    catch (const std::exception& e) {
        Logger::log("Error reverting: " + std::string(e.what()));
        throw;
    }
}

/**
 * @brief Reverts the contents of a directory to a previous state.
 * @param sourceDir The source directory to be reverted.
 * @param destinationDir The destination directory where changes will be reverted.
 */
void MiniVersionControl::revertDirectory(const fs::path& sourceDir, const fs::path& destinationDir) {
    try{
        fs::create_directories(destinationDir);

        for (const auto& entry : fs::directory_iterator(sourceDir)) {
            const fs::path entryPath = entry.path();
            const fs::path destinationPath = destinationDir / entryPath.filename();

            if (fs::is_directory(entryPath)) {
                // Revert directories recursively
                revertDirectory(entryPath, destinationPath);
            } else {
                // Revert individual files
                revertFile(entryPath, destinationPath);
            }
        }
    }
    catch (const std::exception& e) {
        Logger::log("Error reverting directory: " + std::string(e.what()));
        throw;
    }
}

/**
 * @brief Reverts the contents of a file to a previous state.
 * @param source The source file to be reverted.
 * @param destination The destination file where changes will be reverted.
 */
void MiniVersionControl::revertFile(const fs::path& source, const fs::path& destination) {
    try {
        std::lock_guard<std::mutex> lock(MiniVersionControl::mutex_);

        if (fs::is_regular_file(source)) {
            std::ifstream inputFile(source, std::ios::binary);
            if (inputFile.is_open()) {
                // Read file content into a buffer
                std::ostringstream contentBuffer;
                contentBuffer << inputFile.rdbuf();
                inputFile.close();
                // Calculate checksum for the content (excluding just the last 4 bytes)
                uint32_t calculatedChecksum = computeChecksum(contentBuffer.str().substr(0, contentBuffer.str().size() - 4));
                // Extract stored checksum from the last 4 bytes
                uint32_t storedChecksum;
                std::memcpy(&storedChecksum, contentBuffer.str().c_str() + contentBuffer.str().size() - 4, sizeof(storedChecksum));
                // Compare checksums
                if (calculatedChecksum != storedChecksum) {
                    // Log an error and return without reverting
                    Logger::log("Checksum validation failed for file: " + source.filename().string()+ " (skipping revert) some changes may have been lost.");
                    return;
                }
                // Write the content back to the file (excluding just the last 4 bytes)
                std::ofstream outputFile(destination, std::ios::binary);
                if (!outputFile.is_open()) {
                    std::string errorMessage = "Error opening destination file: " + destination.filename().string();
                    Logger::log(errorMessage);
                    throw std::runtime_error(errorMessage);
                }

                outputFile << contentBuffer.str().substr(4, contentBuffer.str().size() - 8);
                outputFile.close();
            } else {
                std::string errorMessage = "Error opening source file: " + source.string();
                Logger::log(errorMessage);
                throw std::runtime_error(errorMessage);
            }
        }
    } catch (const std::exception& e) {
        Logger::log("Error reverting file: " + std::string(e.what()));
        throw;
    }
}


/**
 * @brief Lists all files and directories in the current directory.
 * @return std::vector<std::string> - A vector of file and directory names.
 */
std::vector<std::string> MiniVersionControl::listFilesAndFolders() {
    std::vector<std::string> res;
    for (const auto& entry : fs::directory_iterator(".")) {
        fs::path filePath = entry.path();
        std::string filename = filePath.filename().string();
        if (filename != "main.exe" && filename != ".git" && filename != "log.txt") {
            res.push_back(filename);
        }
    }
    return res;
}


/**
 * @brief Lists all files and directories in the staging area.
 * @return std::vector<std::string> - A vector of file and directory names.
 */
std::vector<std::string> MiniVersionControl::listStagingArea() {
    std::vector<std::string> result;

    try {
        const fs::path stagingPath = ".git/staging";

        if (fs::exists(stagingPath)) {
            for (const auto& entry : fs::directory_iterator(stagingPath)) {
                result.push_back(entry.path().filename().string());
            }
        } else {
            std::cerr << "Staging area not found." << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error listing staging area: " << e.what() << std::endl;
    }

    return result;
}


/**
 * @brief Lists all available versions (commits) in the repository.
 * @return std::vector<std::string> - A vector of version (commit) folder names.
 */
std::vector<std::string> MiniVersionControl::listVersions() {
    try {
        std::vector<std::string> versionList;

        for (const auto& entry : fs::directory_iterator(".git/commits")) {
            versionList.push_back(entry.path().filename().string());
        }

        return versionList;
    } catch (const std::exception& e) {
        Logger::log("Error listing available versions: " + std::string(e.what()));
        throw;
    }
}


/**
 * @brief Deletes a file or directory from the staging area.
 * @param name The name of the file or directory to be deleted.
 */
void MiniVersionControl::deleteFromStaging(const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex_); // Lock the mutex for this scope

    try {
        const fs::path stagingPath = ".git/staging";

        if (fs::exists(stagingPath)) {
            fs::path itemPath = stagingPath / name;

            if (fs::exists(itemPath)) {
                if (fs::is_directory(itemPath)) {
                    // Remove entire folder
                    fs::remove_all(itemPath);
                } else {
                    // Remove file
                    fs::remove(itemPath);
                }
            } else {
                Logger::log("Item '" + name + "' not found in the staging area.");
            }
        } else {
            Logger::log("Staging area not found.");
        }
    } catch (const std::exception& e) {
        Logger::log("Error deleting from staging area: " +  std::string(e.what()));
    }
}


/**
 * @brief Adds a directory to the staging area with enhanced performance using multithreading.
 * @param source The source directory path.
 * @param destination The destination directory path in the staging area.
 */
void MiniVersionControl::enhancedAddDirectoy(const fs::path& source, const fs::path& destination) {
    try {
        fs::create_directory(destination);
        std::vector<std::future<void>> futures;  // Use vector to store futures for threads

        const int maxThreads = 4;  // Set the maximum number of threads

        for (const auto& entry : fs::recursive_directory_iterator(source)) {
            fs::path nestedDestination = destination / entry.path().lexically_relative(source);

            if (entry.is_regular_file()) {
                // Launch a new thread only if the number of active threads is less than the maximum
                if (futures.size() < maxThreads) {
                    // Use std::async to launch the function in a separate thread
                    futures.push_back(std::async(std::launch::async, &MiniVersionControl::addFile, this, entry.path(), nestedDestination));
                } else {
                    // Wait for any thread to finish before launching a new one
                    auto it = std::find_if(futures.begin(), futures.end(), [](const std::future<void>& f) {
                        return f.valid() && f.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
                    });
                    if (it != futures.end()) {
                        it->wait();  // Wait for the thread to finish
                        futures.erase(it);
                    }
                    // Launch a new thread
                    futures.push_back(std::async(std::launch::async, &MiniVersionControl::addFile, this, entry.path(), nestedDestination));
                }
            } else if (entry.is_directory()) {
                // Reuse the current thread for handling directories
                addDirectory(entry.path(), nestedDestination);
            }
        }

        // Wait for all remaining threads to finish
        for (auto& future : futures) {
            future.wait();
        }
    } catch (const std::exception& e) {
        Logger::log("Error adding directory: " + std::string(e.what()));
        throw;
    }

}
