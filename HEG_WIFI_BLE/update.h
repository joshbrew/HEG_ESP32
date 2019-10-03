const char update_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
  <h4>Upload compiled sketch .bin file</h4>
  <form method='POST' action='/doUpdate' enctype='multipart/form-data'>
    <input type='file' name='update'><input type='submit' value='Update'>
  </form>
</html>
)=====";

