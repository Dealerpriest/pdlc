<template>
  <div id="app">
    <router-view></router-view>
  </div>
</template>

<script lang="ts">
import Vue from "vue";
import { initWebSocket } from "./ts/socket";

export default Vue.extend({
  name: "app",
  mounted() {
    initWebSocket();
  },
  components: {}
});
</script>

<style lang="scss">
body {
  margin: 0px;
}
#app {
  font-family: "Avenir", Helvetica, Arial, sans-serif;
  -webkit-font-smoothing: antialiased;
  -moz-osx-font-smoothing: grayscale;
  text-align: center;
  color: #2c3e50;
  // margin-top: 60px;
}
</style>
