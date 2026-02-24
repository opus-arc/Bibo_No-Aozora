#include <string>

#include "utils/internal/MyEssentia/MyEssentia.h"

#define MY_ESSENTIA_AUDIO_FOLDER "../utils/internal/MyEssentia/audio/"
#define MY_ESSENTIA_RESULT_FOLDER "../utils/internal/MyEssentia/results/"

int main() {
    const MyEssentia ess;
    ess.extractFolderToCsv(MY_ESSENTIA_AUDIO_FOLDER, MY_ESSENTIA_RESULT_FOLDER);
    return 0;
}