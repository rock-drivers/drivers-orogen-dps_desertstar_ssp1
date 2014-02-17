#! /usr/bin/env ruby
# -*- coding: utf-8 -*-

require 'orocos'
include Orocos
Orocos::CORBA.name_service = "192.168.128.20"
Orocos.initialize

Orocos.run 'dps' do
  Orocos.log_all_ports
  dps = TaskContext.get 'dps'

  dps.device = '/dev/ttyS1'

  dps.filter_width = 30;

  dps.configure
  dps.start

  loop do
	sleep 0.01
  end 
end

