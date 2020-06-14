
(function ($) {

  Drupal.behaviors.izvestaj_list = {
    attach: function (context) {

      var siteurl = Drupal.settings.izvestaj_list.siteurl;

      $(".izv_pregled").fancybox({
        "width": 600, // or whatever
        "height": 320,
        "type": "iframe"
      });


      $.fancyConfirm = function( opts ) {
        opts  = $.extend( true, {
          title     : "Are you sure?",
          message   : "",
          okButton  : "OK",
          noButton  : "Cancel",
          callback  : $.noop
        }, opts || {} );

        $.fancybox.open({
            type : "html",
            src  :
              "<div class='fc-content'>" +
              "<p>"  + opts.message + "</p>" +
              "<p class='tright' style='text-align: center'>" +
              "<button data-value='0' data-fancybox-close style='padding: 5px 10px; margin:0 10px;'>" + opts.noButton + "</button>" +
              "<button data-value='1' data-fancybox-close style='padding: 5px 10px; margin:0 10px;'>" + opts.okButton + "</button>" +
              "</p>" +
              "</div>",
            opts : {
              animationDuration : 200,
              //animationEffect   : "material",
              modal : true,
              baseTpl :
                '<div class="fancybox-container fc-container" role="dialog" tabindex="-1">' +
                '<div class="fancybox-bg"></div>' +
                '<div class="fancybox-inner">' +
                '<div class="fancybox-stage"></div>' +
                "</div>" +
                "</div>",
              afterClose : function( instance, current, e ) {
                var button = e ? e.target || e.currentTarget : null;
                var value  = button ? $(button).data("value") : 0;
                opts.callback( value );
              }
            }
          }
        );
      };

      $(".izv_delete").click(function(e) {
        e.preventDefault();
        var id=$(this).attr("ref");
        // Open customized confirmation dialog window
        $.fancyConfirm({
          title     : "Potvrda brisanja",
          message   : "Da li stvarno želiš da obrišeš izveštaj?",
          okButton  : "Da",
          noButton  : "Ne",
          callback  : function (value) {
            if (value) {
              $.ajax({
                "async": false,
                "global": false,
                "url":  siteurl + "/opstine_report_delete/"+id,
                "dataType": "json",
                "success": function (data) {
                  console.log(data);
                  location.reload();
                }
              });
            } else {
              console.log("odustao "+id);
            }
          }
        });

      });


    }
  };
})(jQuery);
