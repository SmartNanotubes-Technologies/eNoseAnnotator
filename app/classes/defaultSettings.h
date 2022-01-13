#ifndef DEFAULTVALUES_H
#define DEFAULTVALUES_H

#include <QColor>

// debug settings
#define CLEAR_SETTINGS false

// directories
#define DATA_DIR_KEY "directories/dataDir"
#define DEFAULT_DATA_DIR "."

#define EXPORT_DIR_KEY "directories/exportDir"
#define DEFAULT_EXPORT_DIR "."

#define CLASSIFIER_DIR_KEY "directories/classifierDir"
#define DEFAULT_CLASSIFIER_DIR "."

#define PRESET_DIR_KEY "directories/presetDir"
#define DEFAULT_PRESET_DIR "./presets/"
#define PRESET_SOURCE "../share/presets/"

// limits
#define USE_LIMITS_KEY "settings/useLimits"
#define DEFAULT_USE_LIMITS false
#define UPPER_LIMIT_KEY "settings/upperLimit"
#define DEFAULT_UPPER_LIMIT 90000.0
#define LOWER_LIMIT_KEY "settings/lowerLimit"
#define DEFAULT_LOWER_LIMIT 300.0

// colors
# define GRAPH_BACKGROUND_COLOR QColor(255,250,240)

// curve fit settings
#define CVWIZ_DEFAULT_MODEL_TYPE LeastSquaresFitter::Type::SUPERPOS
#define CVWIZ_DEFAULT_JUMP_BASE_THRESHOLD 0.2
#define CVWIZ_DEFAULT_JUMP_FACTOR 2.0
#define CVWIZ_DEFAULT_RECOVERY_FACTOR 0.25
#define CVWIZ_DEFAULT_BUFFER_SIZE 15
#define CVWIZ_DEFAULT_DETECT_EXPOSITION_START true
#define CVWIZ_DEFAULT_DETECT_RECOVERY_START false
#define CVWIZ_DEFAULT_RECOVERY_TIME 30
#define CVWIZ_DEBUG_MODE false  // true: curve fit executed in a single thread and additional debugging info activated

// functionalisation
#define FUNC_MAX_VALUE 100000
#define FUNC_NC_VALUE 999

// python
#define PYTHON_DIR_KEY "settings/pythonDir"
#define PYTHON_CMD_KEY "settings/pythonCmd"

// devices
#define DEVICE_TIMEOUT 6

// uploader
#define UPLOADER_CMD_TIMEOUT 90000
#define UPLOADER_CMD_SYNC_PERIOD 2*60000
#define INDUSTRY_SELECTION_TEXT "Please select"
#define SHOW_UPLOAD_FEATURE_KEY "settings/show_upload"
#define SHOW_UPLOAD_FEATURE_DEFAULT false
#define SHOW_UPLOAD_LOG_KEY "settings/show_upload_log"
#define SHOW_UPLOAD_LOG_DEFAULT true

// smell annotations
#define SMELL_LIST_KEY "settings/smells"
#define DEFAULT_SMELL_LIST "No Smell"
#define SMELL_SEPARATOR ";"

// running meas autosave
#define RUN_AUTO_SAVE_KEY "settings/run_auto_save_key"
#define DEFAULT_RUN_AUTO_SAVE false
#define RUN_AUTO_SAVE_INTERVAL_KEY "settings/run_auto_save_interval"
#define DEFAULT_RUN_AUTO_SAVE_INTERVAL 5

#endif // DEFAULTVALUES_H
