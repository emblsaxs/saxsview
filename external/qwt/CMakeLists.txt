project (QWT CXX)

qt4_wrap_cpp (MOC qwt_abstract_slider.h qwt_analog_clock.h qwt_compass.h
                  qwt_counter.h qwt_dial.h qwt_dyngrid_layout.h qwt_knob.h
                  qwt_legend.h qwt_legend_item.h qwt_magnifier.h
                  qwt_panner.h qwt_picker.h qwt_plot.h qwt_plot_canvas.h
                  qwt_plot_magnifier.h qwt_plot_panner.h qwt_plot_picker.h
                  qwt_plot_zoomer.h qwt_scale_widget.h qwt_slider.h
                  qwt_text_label.h qwt_thermo.h qwt_wheel.h)

set (HEADERS qwt.h qwt_dial_needle.h qwt_paint_buffer.h qwt_plot_magnifier.h
             qwt_scale_div.h qwt_abstract_scale.h qwt_double_interval.h
             qwt_painter.h qwt_plot_marker.h qwt_scale_draw.h
             qwt_abstract_scale_draw.h qwt_double_range.h qwt_panner.h
             qwt_plot_panner.h qwt_scale_engine.h qwt_abstract_slider.h
             qwt_double_rect.h qwt_picker.h qwt_plot_picker.h qwt_scale_map.h
             qwt_analog_clock.h qwt_dyngrid_layout.h qwt_picker_machine.h
             qwt_plot_printfilter.h qwt_scale_widget.h qwt_array.h
             qwt_event_pattern.h qwt_plot.h qwt_plot_rasteritem.h
             qwt_series_data.h qwt_arrow_button.h qwt_global.h
             qwt_plot_canvas.h qwt_plot_rescaler.h qwt_slider.h qwt_clipper.h
             qwt_interval_symbol.h qwt_plot_curve.h qwt_plot_scaleitem.h
             qwt_spline.h qwt_color_map.h qwt_knob.h qwt_plot_dict.h
             qwt_plot_seriesitem.h qwt_symbol.h qwt_column_symbol.h
             qwt_layout_metrics.h qwt_plot_directpainter.h
             qwt_plot_spectrogram.h qwt_text.h qwt_compass.h qwt_legend.h
             qwt_plot_grid.h qwt_plot_svgitem.h qwt_text_engine.h
             qwt_compass_rose.h qwt_legend_item.h qwt_plot_histogram.h
             qwt_plot_zoomer.h qwt_text_label.h qwt_counter.h
             qwt_legend_itemmanager.h qwt_plot_intervalcurve.h qwt_polygon.h
             qwt_thermo.h qwt_curve_fitter.h qwt_magnifier.h qwt_plot_item.h
             qwt_raster_data.h qwt_valuelist.h qwt_dial.h qwt_math.h
             qwt_plot_layout.h qwt_round_scale_draw.h qwt_wheel.h)


set (SOURCES qwt_abstract_scale.cpp qwt_double_range.cpp
             qwt_picker_machine.cpp qwt_plot_picker.cpp qwt_scale_engine.cpp
             qwt_abstract_scale_draw.cpp qwt_double_rect.cpp qwt_plot.cpp
             qwt_plot_print.cpp qwt_scale_map.cpp qwt_abstract_slider.cpp
             qwt_dyngrid_layout.cpp qwt_plot_axis.cpp qwt_plot_printfilter.cpp
             qwt_scale_widget.cpp qwt_analog_clock.cpp qwt_event_pattern.cpp
             qwt_plot_canvas.cpp qwt_plot_rasteritem.cpp qwt_series_data.cpp
             qwt_arrow_button.cpp qwt_interval_symbol.cpp qwt_plot_curve.cpp
             qwt_plot_rescaler.cpp qwt_slider.cpp qwt_clipper.cpp qwt_knob.cpp
             qwt_plot_dict.cpp qwt_plot_scaleitem.cpp qwt_spline.cpp
             qwt_color_map.cpp qwt_layout_metrics.cpp
             qwt_plot_directpainter.cpp qwt_plot_seriesitem.cpp qwt_symbol.cpp
             qwt_column_symbol.cpp qwt_legend.cpp qwt_plot_grid.cpp
             qwt_plot_spectrogram.cpp qwt_text.cpp qwt_compass.cpp
             qwt_legend_item.cpp qwt_plot_histogram.cpp qwt_plot_svgitem.cpp
             qwt_text_engine.cpp qwt_compass_rose.cpp qwt_magnifier.cpp
             qwt_plot_intervalcurve.cpp qwt_plot_xml.cpp qwt_text_label.cpp
             qwt_counter.cpp qwt_math.cpp qwt_plot_item.cpp qwt_plot_zoomer.cpp
             qwt_thermo.cpp qwt_curve_fitter.cpp qwt_paint_buffer.cpp
             qwt_plot_layout.cpp qwt_raster_data.cpp qwt_wheel.cpp qwt_dial.cpp
             qwt_painter.cpp qwt_plot_magnifier.cpp qwt_round_scale_draw.cpp
             qwt_dial_needle.cpp qwt_panner.cpp qwt_plot_marker.cpp
             qwt_scale_div.cpp qwt_double_interval.cpp qwt_picker.cpp
             qwt_plot_panner.cpp qwt_scale_draw.cpp)

add_library (qwt SHARED ${HEADERS} ${SOURCES} ${MOC})

set_target_properties (qwt PROPERTIES VERSION 5.3.0)

target_link_libraries (qwt ${QT_LIBRARIES})

install (TARGETS qwt
         LIBRARY DESTINATION lib
         RUNTIME DESTINATION bin)
install (FILES ${HEADERS}
         DESTINATION include/qwt)
