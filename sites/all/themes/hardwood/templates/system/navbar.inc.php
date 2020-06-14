<nav id="navigation"
     class="main-header navbar navbar-light bg-faded navbar-full navbar-expand-lg">
    <div class="container">
        <a class="navbar-brand" href="/">
            <span class="logo-icon"><img style="height:48px" src="<?php echo file_create_url(drupal_get_path('theme',$GLOBALS['theme'])); ?>/logo.png" /></span>
            <div class="logo-wrapper">
                <div class="text-muted text-xs hidden-xs-down">
                  <?php print variable_get('site_slogan') ?>
                </div>
            </div>
        </a>

        <button class="navbar-toggler navbar-toggler-right"
                type="button"
                data-toggle="collapse"
                data-target="#navbar-collapse"
                aria-controls="navbarSupportedContent"
                aria-expanded="false"
                aria-label="Toggle navigation">
            <span class="navbar-toggler-icon"></span>
        </button>

        <div class="collapse navbar-collapse" id="navbar-collapse">
          <?php
          global $base_url;
          if (!empty($primary_nav)):
            print render($primary_nav);
          endif;
          ?>

          <?php
          if (!empty($secondary_nav)):
            print render($secondary_nav);
          endif;
          ?>

          <?php if (!user_is_logged_in()): ?>
              <ul class="navbar-nav mr-md-auto mr-lg-0">
                  <li class="leaf nav-item">
                      <a href="<?php echo $base_url;?>/user" class="nav-link">Login</a>
                  </li>
              </ul>
          <?php endif; ?>
        </div>
    </div>
</nav> <!-- /.section, /#navigation -->
