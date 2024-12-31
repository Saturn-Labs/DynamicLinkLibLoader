#pragma once

#include <string>
#include <regex>
#include <stdexcept>

class Semver {
public:
    Semver(const std::string& version) {
        std::regex version_regex(R"((\d+|\*)(\.(\d+|\*)(\.(\d+|\*)(\.(\d+|\*))?)?)?)");
        std::smatch match;
        if (std::regex_match(version, match, version_regex)) {
            major = match[1].str();
            minor = match[3].str().empty() ? "*" : match[3].str();
            patch = match[5].str().empty() ? "*" : match[5].str();
            build = match[7].str().empty() ? "*" : match[7].str();
        } else {
            throw std::invalid_argument("Invalid version format");
        }
    }

    std::string getMajor() const { return major; }
    std::string getMinor() const { return minor; }
    std::string getPatch() const { return patch; }
    std::string getBuild() const { return build; }

    std::string toString() const {
        return major + "." + minor + "." + patch + "." + build;
    }

    bool operator==(const Semver& other) const {
        return compareComponent(major, other.major) == 0 &&
               compareComponent(minor, other.minor) == 0 &&
               compareComponent(patch, other.patch) == 0 &&
               compareComponent(build, other.build) == 0;
    }

    bool operator!=(const Semver& other) const {
        return !(*this == other);
    }

    bool operator<(const Semver& other) const {
        return compareComponent(major, other.major) < 0 ||
               (compareComponent(major, other.major) == 0 && compareComponent(minor, other.minor) < 0) ||
               (compareComponent(major, other.major) == 0 && compareComponent(minor, other.minor) == 0 && compareComponent(patch, other.patch) < 0) ||
               (compareComponent(major, other.major) == 0 && compareComponent(minor, other.minor) == 0 && compareComponent(patch, other.patch) == 0 && compareComponent(build, other.build) < 0);
    }

    bool operator<=(const Semver& other) const {
        return *this < other || *this == other;
    }

    bool operator>(const Semver& other) const {
        return !(*this <= other);
    }

    bool operator>=(const Semver& other) const {
        return !(*this < other);
    }

    void incrementMajor() {
        if (major != "*") {
            major = std::to_string(std::stoi(major) + 1);
        }
    }

    void incrementMinor() {
        if (minor != "*") {
            minor = std::to_string(std::stoi(minor) + 1);
        }
    }

    void incrementPatch() {
        if (patch != "*") {
            patch = std::to_string(std::stoi(patch) + 1);
        }
    }

    void incrementBuild() {
        if (build != "*") {
            build = std::to_string(std::stoi(build) + 1);
        }
    }

	bool fullWildcard() const {
		return major == "*" && minor == "*" && patch == "*" && build == "*";
	}
private:
    std::string major;
    std::string minor;
    std::string patch;
    std::string build;

    int compareComponent(const std::string& a, const std::string& b) const {
        if (a == "*" || b == "*") return 0;
        try {
            return std::stoi(a) - std::stoi(b);
        } catch (const std::invalid_argument&) {
            throw std::invalid_argument("Invalid component format");
        } catch (const std::out_of_range&) {
            throw std::out_of_range("Component value out of range");
        }
    }
};
