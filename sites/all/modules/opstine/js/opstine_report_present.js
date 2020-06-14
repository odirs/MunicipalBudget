(function ($) {
  Drupal.behaviors.opstine_report_present = {
    attach: function (context) {

      var datapath = Drupal.settings.opstine.datapath;
      var siteurl = Drupal.settings.opstine.siteurl;

      var platform_data_file_url = siteurl+"/opstine_report_present_cb";

      $.draw_opstine = function(odata) {
        var present=jQuery.parseJSON(odata);
        //Width and height

        var w = $("#srbija_opstine_map").html("").width();
        if (w>1200) {w=1200}
        var h = w*3/4;

        //Define map projection
        var projection = d3.geo.mercator().translate([0, 0]).scale(1);

        //Define path generator
        var path = d3.geo.path().projection(projection);

        var zoom = d3.behavior.zoom()
          .translate([0, 0])
          .scale(1)
          .scaleExtent([1, 8]);

        //Create SVG element
        var svg = d3.select("#srbija_opstine_map").append("svg").attr("width", w).attr("height", h);

        //Load in GeoJSON data
        d3.json(datapath, function (json) {

          // Calculate bounding box transforms for entire collection
          var b = path.bounds(json),
            s = 0.95 / Math.max((b[1][0] - b[0][0]) / w, (b[1][1] - b[0][1]) / h),
            t = [(w - s * (b[1][0] + b[0][0])) / 2, (h - s * (b[1][1] + b[0][1])) / 2];

          // Update the projection
          projection.scale(s).translate(t);
          //Bind data and create one path per GeoJSON feature
          svg.selectAll("path")
            .data(json.features)
            .enter()
            .append("path")
            .attr("d", path)
            .style("fill", function (d) {
              if(d.properties.NAME_2 in present) {
                return "green";
              } else {
				if(d.properties.NAME_0 !== "Kosovo"){
					return "#ccc";
				} else {
					return "#ccc";
				}
              }
            })
            .on("mouseover", function (d, i) {
              if (typeof present[d.properties.NAME_2] !== "undefined" ){
                d3.select(this).classed("active clickable", true);
              } else {
                d3.select(this).classed("active", true);
              }

              /*$(this).popover({
                placement: "auto",
                container: "body",
                trigger: "manual",
                html : true,
                sanitize: false,
                title : function() {return d.properties.NAME_2;},
                content: function() {
                  return present[d.properties.NAME_2];
                }
              }).popover("show");*/

            })
            .on("mouseout", function (d, i) {
              if (typeof present[d.properties.NAME_2] !== "undefined" ){
                d3.select(this).classed("clickable", false);
                d3.select(this).classed("active", false);
              } else {
                d3.select(this).classed("active", false);
              }
              //$(".popover").each(function() { $(this).remove(); });
            })
            .on("mousemove", function (d, i) {
            }).
            on("click", function (d, i) {
              if (typeof present[d.properties.NAME_2] !== "undefined" ){
                //window.location.href=siteurl+"/opstine/"+d.properties.TID;
                $(".popover").each(function() { $(this).remove(); });
                $(this).popover({
                  placement: "auto",
                  container: "body",
                  trigger: "manual",
                  html : true,
                  sanitize: false,
                  title : function() {return d.properties.NAME_2;},
                  content: function() {
                    return (present[d.properties.NAME_2]+" "+"<a href='"+siteurl+"/opstine/"+d.properties.TID+"'>R1</a>"+" "+
                      "<a href='"+siteurl+"/cp/"+d.properties.TID+"/outcome'>R2</a> "+
                      "<a href='"+siteurl+"/cp/"+d.properties.TID+"/income'>P2</a> "
                    );
                  }
                }).popover("show");
              } else {
                $(".popover").each(function() { $(this).remove(); });
              }
            })
          ;
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
