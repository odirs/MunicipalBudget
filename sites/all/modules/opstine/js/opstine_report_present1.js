(function ($) {
  Drupal.behaviors.opstine_report_present = {
    attach: function (context) {

      var datapath = Drupal.settings.opstine.datapath;
      var siteurl = Drupal.settings.opstine.siteurl;

      var platform_data_file_url = siteurl+"/opstine_report_present_cb";

      $.draw_opstine = function(odata) {
        var present=jQuery.parseJSON(odata);

        var opstine = $.ajax({
          url: datapath,
          dataType: "json",
          success: console.log("County data successfully loaded."),
          error: function(xhr) {
            alert(xhr.statusText);
          }
        });

        $.when(opstine).done(function() {
          var gjopstine;
          var map = L.map("srbija_opstine_map").setView([44, 20], 7);

          function style(d) {
            var color="#ccc";
            if(d.properties.NAME_2 in present) {
              color="#0D4077";
            }
            return {
              fillColor: color,
              weight: 2,
              opacity: 1,
              color: 'white',
              dashArray: '3',
              fillOpacity: 0.8
            };
          }

          L.tileLayer('https://{s}.basemaps.cartocdn.com/rastertiles/voyager_nolabels/{z}/{x}/{y}{r}.png', {
            subdomains: 'abcd',
            maxZoom: 18
          }).addTo(map);

          function resetHighlight(e) {
            gjopstine.resetStyle(e.target);
          }

          function zoomToFeature(e) {
            map.fitBounds(e.target.getBounds());
          }

          function highlightFeature(e) {
            var layer = e.target;
            layer.setStyle({
              fillOpacity: 0.5
            });

            if (!L.Browser.ie && !L.Browser.opera && !L.Browser.edge) {
              layer.bringToFront();
            }
          }

          function onEachFeature(feature, layer) {
            if(feature.properties.NAME_2 in present) {
              layer.bindPopup(
                /*
                  "<h4>"+feature.properties.NAME_2+"</h4>"+
                  ((typeof present[feature.properties.NAME_2] !== "undefined") ?
                    present[feature.properties.NAME_2]+" Prihod:"+
            "<a href='"+siteurl+"/opstine/"+feature.properties.TID+"/income'>1</a>"+" "+
                    "<a href='"+siteurl+"/cp/"+feature.properties.TID+"/outcome'>2</a> Rashod:"+
            "<a href='"+siteurl+"/opstine/"+feature.properties.TID+"/outcome'>1</a>"+" "+
                    "<a href='"+siteurl+"/cp/"+feature.properties.TID+"/income'>2</a> " : "")
                 */
                "<h4>" + feature.properties.NAME_2 + "</h4>" + "<a href='" + siteurl + "/opstine/" + feature.properties.TID + "/income'>ПРИХОД</a>" + "  <a href='" + siteurl + "/opstine/" + feature.properties.TID + "/outcome'>РАСХОД</a>"
              );
            } else {
              layer.bindPopup(
                "<h4>" + feature.properties.NAME_2 + "</h4>"
              );
            }
            layer.on({
              mouseover: highlightFeature,
              mouseout: resetHighlight
            });
          }

          gjopstine = L.geoJSON(opstine.responseJSON, {style: style, onEachFeature: onEachFeature}).addTo(map);

        });

      };

      var gl_data=null;
      $.ajax({
        type: "GET",
        url: platform_data_file_url,
        async: false,
        success: function (data) {
          gl_data=data;
          $.draw_opstine(data);
        }
      });

      $(window).on('resize', function(){
        $.draw_opstine(gl_data);
      });

      //$.draw_opstine();

    }
  };
})(jQuery);
