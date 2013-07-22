#!/usr/bin/env ruby

Vagrant.configure("2") do |config|
  config.vm.box = "ubuntu-1204-cloud-i386"
  config.vm.box_url = "http://cloud-images.ubuntu.com/precise/current/precise-server-cloudimg-vagrant-i386-disk1.box"
  config.vm.network :forwarded_port, guest: 80, host: 8080

  # Boot with a GUI so you can see the screen. (Default is headless)
  # config.vm.boot_mode = :gui

  config.vm.provision :puppet do |puppet|
    puppet.manifests_path = "tools/lin/scripts"
    puppet.manifest_file = "emscripten.pp"
  end
end
