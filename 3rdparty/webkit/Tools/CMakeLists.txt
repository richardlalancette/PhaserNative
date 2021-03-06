set_property(DIRECTORY . PROPERTY FOLDER "Tools")

if ("${PORT}" STREQUAL "GTK")
    if (DEVELOPER_MODE)
      add_subdirectory(WebKitTestRunner)
      add_subdirectory(ImageDiff)
      if (ENABLE_API_TESTS)
          add_subdirectory(TestWebKitAPI/glib)
      endif ()
      if (ENABLE_NETSCAPE_PLUGIN_API)
          add_subdirectory(DumpRenderTree/TestNetscapePlugIn)
      endif ()
    endif ()

    if (ENABLE_MINIBROWSER)
        add_subdirectory(MiniBrowser/gtk)
    endif ()
elseif ("${PORT}" STREQUAL "Mac")
    add_subdirectory(DumpRenderTree)
    add_subdirectory(WebKitTestRunner)
    add_subdirectory(MiniBrowser/mac)
elseif ("${PORT}" STREQUAL "JSCOnly")
elseif ("${PORT}" STREQUAL "WPE")
    if (DEVELOPER_MODE)
        add_subdirectory(ImageDiff)
        add_subdirectory(WebKitTestRunner)
        add_subdirectory(wpe/HeadlessViewBackend)
        if (ENABLE_API_TESTS)
            add_subdirectory(TestWebKitAPI/glib)
        endif ()
    endif ()
elseif ("${PORT}" STREQUAL "Win")
    add_subdirectory(DumpRenderTree)
    add_subdirectory(ImageDiff)
    if (ENABLE_WEBKIT_LEGACY)
        add_subdirectory(MiniBrowser/win)
    endif ()
endif ()

if (ENABLE_API_TESTS)
    add_subdirectory(TestWebKitAPI)
endif ()
