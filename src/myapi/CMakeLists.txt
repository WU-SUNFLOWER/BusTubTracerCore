add_library(
    bustub_myapi
    OBJECT
    api_manager.cpp
    process_record_context.cpp)

set(ALL_OBJECT_FILES
    ${ALL_OBJECT_FILES} $<TARGET_OBJECTS:bustub_myapi>
    PARENT_SCOPE)
