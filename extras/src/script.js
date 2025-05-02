document.addEventListener('DOMContentLoaded', function() {
  // Navigation functionality
  const menuItems = document.querySelectorAll('.menu-section ul li a');
  const contentSections = document.querySelectorAll('.content-section');

  menuItems.forEach(item => {
      item.addEventListener('click', (e) => {
          e.preventDefault();
          
          // Remove active class from all menu items
          menuItems.forEach(i => i.classList.remove('active'));
          // Add active class to clicked menu item
          item.classList.add('active');

          // Hide all content sections
          contentSections.forEach(section => {
              section.classList.remove('active');
          });

          // Show the selected content section
          const targetId = item.getAttribute('href').substring(1);
          const targetSection = document.getElementById(targetId);
          if (targetSection) {
              targetSection.classList.add('active');
          }
      });
  });

  // Node navigation functionality
  const nodeButtons = document.querySelectorAll('.node-btn');
  const nodeData = {
      '0x01': [
          { address: '0x0000', key: 'Voltage value', value: '220' },
          { address: '0x0001', key: 'Current value', value: '1' },
          { address: '0x0002', key: 'Power value', value: '220' },
          { address: '0x0003', key: 'Energy value', value: '10' },
          { address: '0x0004', key: 'Frequency value', value: '50' }
      ],
      '0x02': [
          { address: '0x0000', key: 'Voltage value', value: '221' },
          { address: '0x0001', key: 'Current value', value: '1.2' },
          { address: '0x0002', key: 'Power value', value: '225' },
          { address: '0x0003', key: 'Energy value', value: '12' },
          { address: '0x0004', key: 'Frequency value', value: '50' }
      ],
      // Add data for other nodes as needed
  };

  nodeButtons.forEach(button => {
      button.addEventListener('click', () => {
          // Update active state of buttons
          nodeButtons.forEach(btn => btn.classList.remove('active'));
          button.classList.add('active');

          // Update table data based on selected node
          const nodeId = button.textContent;
          const data = nodeData[nodeId] || nodeData['0x01']; // Default to first node if no data
          updateNodeTable(data);
      });
  });

  function updateNodeTable(data) {
      const tbody = document.querySelector('#map-ping .node-navigation + table tbody');
      if (tbody) {
          tbody.innerHTML = data.map(item => `
              <tr>
                  <td>${item.address}</td>
                  <td>${item.key}</td>
                  <td>${item.value}</td>
              </tr>
          `).join('');
      }
  }

  // Tab switching functionality
  const tabButtons = document.querySelectorAll('.tab-btn');
  const tabContents = document.querySelectorAll('.tab-content');

  tabButtons.forEach(button => {
      button.addEventListener('click', () => {
          // Remove active class from all buttons and contents
          tabButtons.forEach(btn => btn.classList.remove('active'));
          tabContents.forEach(content => content.style.display = 'none');

          // Add active class to clicked button
          button.classList.add('active');

          // Show corresponding content
          const tabId = button.getAttribute('data-tab');
          document.getElementById(`${tabId}-tab`).style.display = 'block';
      });
  });

  // NTP Server management
  const ntpServersContainer = document.querySelector('.ntp-servers');
  const addServerBtn = document.querySelector('.add-server-btn');

  if (addServerBtn) {
      addServerBtn.addEventListener('click', () => {
          const serverEntry = document.createElement('div');
          serverEntry.className = 'server-entry';
          serverEntry.innerHTML = `
              <input type="text" value="">
              <button class="remove-btn">-</button>
          `;
          ntpServersContainer.insertBefore(serverEntry, addServerBtn);

          // Add event listener to new remove button
          const removeBtn = serverEntry.querySelector('.remove-btn');
          removeBtn.addEventListener('click', () => {
              serverEntry.remove();
          });
      });
  }

  // Add event listeners to existing remove buttons
  document.querySelectorAll('.remove-btn').forEach(button => {
      button.addEventListener('click', () => {
          button.parentElement.remove();
      });
  });

  // Save button functionality
  const saveBtn = document.querySelector('.save-btn');
  if (saveBtn) {
      saveBtn.addEventListener('click', () => {
          // Collect all form data
          const formData = {
              hostname: document.querySelector('input[value="esp8266"]').value,
              timezone: document.querySelector('input[value="UTC"]').value,
              ntpEnabled: document.querySelector('input[type="checkbox"]').checked,
              ntpServers: Array.from(document.querySelectorAll('.server-entry input')).map(input => input.value)
          };

          // You would typically send this data to a server
          console.log('Saving configuration:', formData);
          alert('Configuration saved successfully!');
      });
  }

  // Reset button functionality
  const resetBtn = document.querySelector('.reset-btn');
  if (resetBtn) {
      resetBtn.addEventListener('click', () => {
          if (confirm('Are you sure you want to reset all settings?')) {
              // Reset all input fields to their default values
              document.querySelectorAll('input[type="text"]').forEach(input => {
                  input.value = input.defaultValue;
              });
              document.querySelectorAll('input[type="checkbox"]').forEach(checkbox => {
                  checkbox.checked = checkbox.defaultChecked;
              });
          }
      });
  }

  // Sync with NTP Server button
  const syncBtn = document.querySelector('.sync-btn');
  if (syncBtn) {
      syncBtn.addEventListener('click', () => {
          // Simulate NTP sync
          const now = new Date();
          document.querySelector('input[value*="2024"]').value = now.toLocaleString();
          alert('Time synchronized with NTP server');
      });
  }

  // Menu section highlighting
  const menuHeaders = document.querySelectorAll('.menu-header');
  menuHeaders.forEach(header => {
      header.addEventListener('click', () => {
          menuHeaders.forEach(h => h.classList.remove('active'));
          header.classList.add('active');
      });
  });

  // Admin Save/Reset button functionality
  const adminSaveBtn = document.getElementById('admin-save-btn');
  const adminResetBtn = document.getElementById('admin-reset-btn');
  const adminUsername = document.getElementById('admin-username');
  const adminPassword = document.getElementById('admin-password');

  if (adminSaveBtn && adminResetBtn && adminUsername && adminPassword) {
      adminSaveBtn.addEventListener('click', () => {
          alert(`Saved!\nUsername: ${adminUsername.value}\nPassword: ${adminPassword.value}`);
      });
      adminResetBtn.addEventListener('click', () => {
          adminUsername.value = 'admin';
          adminPassword.value = 'admin';
      });
  }

  // WAN Save/Reset button functionality
  const wanSaveBtn = document.getElementById('wan-save-btn');
  const wanResetBtn = document.getElementById('wan-reset-btn');
  const wanDhcp1 = document.getElementById('wan-dhcp1');
  const wanDhcp2 = document.getElementById('wan-dhcp2');
  const wanIp = document.getElementById('wan-ip');
  const wanMask = document.getElementById('wan-mask');
  const wanGateway = document.getElementById('wan-gateway');
  const wanDns = document.getElementById('wan-dns');

  if (wanSaveBtn && wanResetBtn) {
      wanSaveBtn.addEventListener('click', () => {
          alert(`Saved!\nDHCP: ${wanDhcp1.value}, ${wanDhcp2.value}\nWan IP: ${wanIp.value}\nSubnet Mask: ${wanMask.value}\nGateway: ${wanGateway.value}\nDNS: ${wanDns.value}`);
      });
      wanResetBtn.addEventListener('click', () => {
          wanDhcp1.value = 'True';
          wanDhcp2.value = 'True';
          wanIp.value = '10.10.1.34';
          wanMask.value = '255.255.255.0';
          wanGateway.value = '10.10.1.1';
          wanDns.value = '8.8.8.8';
      });
  }

  // LAN Save/Reset button functionality
  const lanSaveBtn = document.getElementById('lan-save-btn');
  const lanResetBtn = document.getElementById('lan-reset-btn');
  const lanIp = document.getElementById('lan-ip');
  const lanMask = document.getElementById('lan-mask');
  const lanDhcp1 = document.getElementById('lan-dhcp1');
  const lanDhcp2 = document.getElementById('lan-dhcp2');

  if (lanSaveBtn && lanResetBtn) {
      lanSaveBtn.addEventListener('click', () => {
          alert(`Saved!\nLan IP: ${lanIp.value}\nSubnet Mask: ${lanMask.value}\nDHCP Server: ${lanDhcp1.value}, ${lanDhcp2.value}`);
      });
      lanResetBtn.addEventListener('click', () => {
          lanIp.value = '10.10.2.1';
          lanMask.value = '255.255.255.0';
          lanDhcp1.value = 'True';
          lanDhcp2.value = 'False';
      });
  }

  // Wi-Fi Save/Reset button functionality
  const wifiSaveBtn = document.getElementById('wifi-save-btn');
  const wifiResetBtn = document.getElementById('wifi-reset-btn');
  const wifiMode = document.getElementById('wifi-mode');
  const staSsid = document.getElementById('sta-ssid');
  const staKey = document.getElementById('sta-key');
  const wifiMask = document.getElementById('wifi-mask');
  const wifiDhcp = document.getElementById('wifi-dhcp');
  const apSsid = document.getElementById('ap-ssid');
  const apKey = document.getElementById('ap-key');
  const apChannel = document.getElementById('ap-channel');

  if (wifiSaveBtn && wifiResetBtn) {
      wifiSaveBtn.addEventListener('click', () => {
          alert(`Saved!\nWiFi Mode: ${wifiMode.value}\nSTA SSID: ${staSsid.value}\nSTA Key: ${staKey.value}\nSubnet Mask: ${wifiMask.value}\nDHCP Server: ${wifiDhcp.value}\nAP SSID: ${apSsid.value}\nAP Key: ${apKey.value}\nAP Channel: ${apChannel.value}`);
      });
      wifiResetBtn.addEventListener('click', () => {
          wifiMode.value = 'Gateway';
          staSsid.value = 'MIND';
          staKey.value = '12345678';
          wifiMask.value = '255.255.255.0';
          wifiDhcp.value = 'True';
          apSsid.value = 'MIND';
          apKey.value = '12345678';
          apChannel.value = 'AUTO';
      });
  }

  // Communication Save/Reset button functionality
  const commSaveBtn = document.getElementById('comm-save-btn');
  const commResetBtn = document.getElementById('comm-reset-btn');
  const commFields = [
      'comm-proto1','comm-proto2','comm-proto3',
      'comm-server','comm-server2','comm-port1','comm-port2','comm-buf1','comm-buf2','comm-keep1','comm-keep2','comm-timeout1','comm-timeout2',
      'comm-connect1','comm-connect2','comm-reg1','comm-reg2','comm-mqttver1','comm-mqttver2','comm-client1','comm-client2','comm-acc1','comm-acc2','comm-pass1','comm-pass2','comm-heart1','comm-heart2','comm-heartcode1','comm-heartcode2','comm-hearttime1','comm-hearttime2','comm-maxaccept1','comm-maxaccept2','comm-topic1','comm-topic2','comm-qos1','comm-qos2',
      'comm-pubtopic','comm-pubqos','comm-pingperiod'
  ];
  const commDefaults = {
      'comm-proto1': 'Things board', 'comm-proto2': 'MQTT', 'comm-proto3': 'Modbus TCP',
      'comm-server': 'localhost', 'comm-server2': '',
      'comm-port1': '1883', 'comm-port2': '502',
      'comm-buf1': '512', 'comm-buf2': '512',
      'comm-keep1': '60', 'comm-keep2': '60',
      'comm-timeout1': '0', 'comm-timeout2': '0',
      'comm-connect1': 'Always', 'comm-connect2': 'Always',
      'comm-reg1': 'Disable', 'comm-reg2': 'Disable',
      'comm-mqttver1': '3', 'comm-mqttver2': '3',
      'comm-client1': 'xxxxxxxxx', 'comm-client2': 'xxxxxxxxx',
      'comm-acc1': 'xxxxxxxxx', 'comm-acc2': 'xxxxxxxxx',
      'comm-pass1': 'xxxxxxxxx', 'comm-pass2': 'xxxxxxxxx',
      'comm-heart1': 'True', 'comm-heart2': 'True',
      'comm-heartcode1': 'xxxxxxxxx', 'comm-heartcode2': 'xxxxxxxxx',
      'comm-hearttime1': '60', 'comm-hearttime2': '60',
      'comm-maxaccept1': '', 'comm-maxaccept2': '3',
      'comm-topic1': 'xxxxxxxxx', 'comm-topic2': 'xxxxxxxxx',
      'comm-qos1': '0', 'comm-qos2': '0',
      'comm-pubtopic': 'xxxxxxxxx', 'comm-pubqos': '0', 'comm-pingperiod': '0'
  };
  if (commSaveBtn && commResetBtn) {
      commSaveBtn.addEventListener('click', () => {
          let msg = 'Saved!';
          commFields.forEach(f => {
              const el = document.getElementById(f);
              if (el) msg += `\n${f}: ${el.value}`;
          });
          alert(msg);
      });
      commResetBtn.addEventListener('click', () => {
          commFields.forEach(f => {
              const el = document.getElementById(f);
              if (el) el.value = commDefaults[f];
          });
      });
  }

  // Console Refresh/Clear button functionality
  const consoleRefreshBtn = document.getElementById('console-refresh-btn');
  const consoleClearBtn = document.getElementById('console-clear-btn');
  const consoleLog = document.getElementById('console-log');
  if (consoleRefreshBtn && consoleClearBtn && consoleLog) {
      consoleRefreshBtn.addEventListener('click', () => {
          const now = new Date();
          const fakeLog = `MS;P0=${Math.floor(Math.random()*10000)};P1=${Math.floor(Math.random()*10000)};T=${now.toLocaleTimeString()};<br>`;
          consoleLog.innerHTML += fakeLog;
          consoleLog.scrollTop = consoleLog.scrollHeight;
      });
      consoleClearBtn.addEventListener('click', () => {
          consoleLog.innerHTML = '';
      });
  }

  // Scheduled Tasks Save/Reset button functionality
  const tasksSaveBtn = document.getElementById('tasks-save-btn');
  const tasksResetBtn = document.getElementById('tasks-reset-btn');
  const taskType = document.getElementById('task-type');
  const taskName = document.getElementById('task-name');
  const taskCycle = document.getElementById('task-cycle');
  const taskTopic = document.getElementById('task-topic');
  const taskFrame = document.getElementById('task-frame');
  if (tasksSaveBtn && tasksResetBtn) {
      tasksSaveBtn.addEventListener('click', () => {
          alert(`Saved!\nTask type: ${taskType.value}\nTask name: ${taskName.value}\nExecute cycle: ${taskCycle.value}\nTopic: ${taskTopic.value}\nFrame: ${taskFrame.value}`);
      });
      tasksResetBtn.addEventListener('click', () => {
          taskType.value = 'Send MQTT';
          taskName.value = 'Send device attribute';
          taskCycle.value = 'Cron';
          taskTopic.value = 'v1/gateway/attributes';
          taskFrame.value = '{ "Device A": {  "attribute1":"value1",  "attribute2": 42 }, "Device B": {  "attribute1": "value1",  "attribute2": 42 }}';
      });
  }
  // Edit/Delete task row (giả lập)
  document.querySelectorAll('.edit-task').forEach(btn => {
      btn.addEventListener('click', (e) => {
          e.preventDefault();
          alert('Edit task (demo)');
      });
  });
  document.querySelectorAll('.delete-task').forEach(btn => {
      btn.addEventListener('click', (e) => {
          e.preventDefault();
          alert('Delete task (demo)');
      });
  });

  // Add/Back button logic for Scheduled Tasks (luôn hoạt động)
  const tasksAddBtn = document.getElementById('tasks-add-btn');
  const tasksForm = document.getElementById('tasks-form');
  const tasksBackBtn = document.getElementById('tasks-back-btn');
  const tasksListTableWrapper = document.getElementById('tasks-list-table-wrapper');
  if (tasksAddBtn && tasksForm && tasksBackBtn && tasksListTableWrapper) {
      tasksAddBtn.addEventListener('click', () => {
          tasksForm.style.display = '';
          tasksListTableWrapper.style.display = 'none';
          tasksAddBtn.style.display = 'none';
      });
      tasksBackBtn.addEventListener('click', () => {
          tasksForm.style.display = 'none';
          tasksListTableWrapper.style.display = '';
          tasksAddBtn.style.display = '';
      });
  }

  // Modbus Poll Save/Reset button functionality
  const modbusSaveBtn = document.getElementById('modbus-poll-save-btn');
  const modbusResetBtn = document.getElementById('modbus-poll-reset-btn');
  const modbusSlaveId = document.getElementById('modbus-slave-id');
  const modbusAddressMode = document.getElementById('modbus-address-mode');
  const modbusFuntion = document.getElementById('modbus-funtion');
  const modbusAddress = document.getElementById('modbus-address');
  const modbusQuantity = document.getElementById('modbus-quantity');
  if (modbusSaveBtn && modbusResetBtn) {
      modbusSaveBtn.addEventListener('click', () => {
          alert(`Saved!\nSlave ID: ${modbusSlaveId.value}\nAddress Mode: ${modbusAddressMode.value}\nFuntion: ${modbusFuntion.value}\nAddress: ${modbusAddress.value}\nQuantity: ${modbusQuantity.value}`);
      });
      modbusResetBtn.addEventListener('click', () => {
          modbusSlaveId.value = '0';
          modbusAddressMode.value = 'DEC';
          modbusFuntion.value = '4x';
          modbusAddress.value = '0';
          modbusQuantity.value = '10';
      });
  }

  // Modbus Poll Node navigation and edit/delete logic
  const modbusNodeBtns = document.querySelectorAll('.modbus-node-btn');
  const modbusNodeTbody = document.getElementById('modbus-node-tbody');
  const modbusNodeData = {
      '0x01': [
          { group: 0, addr: '0x0000', key: 'Voltage value', value: 220 },
          { group: 0, addr: '0x0001', key: 'Current value', value: 1 },
          { group: 0, addr: '0x0002', key: 'Power value', value: 220 },
          { group: 0, addr: '0x0003', key: 'Energy value', value: 10 }
      ],
      '0x02': [
          { group: 0, addr: '0x0000', key: 'Voltage value', value: 221 },
          { group: 0, addr: '0x0001', key: 'Current value', value: 1.2 },
          { group: 0, addr: '0x0002', key: 'Power value', value: 225 },
          { group: 0, addr: '0x0003', key: 'Energy value', value: 12 }
      ],
      // Có thể thêm các node khác nếu muốn
  };
  function renderModbusNodeTable(nodeId) {
      const data = modbusNodeData[nodeId] || modbusNodeData['0x01'];
      modbusNodeTbody.innerHTML = data.map(item => `
          <tr>
              <td>${item.group}</td><td>${item.addr}</td><td>${item.key}</td><td>${item.value}</td>
              <td><a href="#" class="edit-modbus">edit</a> <a href="#" class="delete-modbus" style="color:red;">delete</a></td>
          </tr>
      `).join('');
      attachModbusEditDelete();
  }
  function attachModbusEditDelete() {
      modbusNodeTbody.querySelectorAll('.edit-modbus').forEach(btn => {
          btn.onclick = function(e) {
              e.preventDefault();
              alert('Edit modbus row (demo)');
          };
      });
      modbusNodeTbody.querySelectorAll('.delete-modbus').forEach(btn => {
          btn.onclick = function(e) {
              e.preventDefault();
              alert('Delete modbus row (demo)');
          };
      });
  }
  modbusNodeBtns.forEach(btn => {
      btn.addEventListener('click', () => {
          modbusNodeBtns.forEach(b => b.classList.remove('active'));
          btn.classList.add('active');
          renderModbusNodeTable(btn.textContent);
      });
  });
  renderModbusNodeTable('0x01');

  // Modbus Poll Node Add/Back/Save/Reset form logic
  const modbusNodeAddBtn = document.getElementById('modbus-node-add-btn');
  const modbusNodeForm = document.getElementById('modbus-node-form');
  const modbusNodeBackBtn = document.getElementById('modbus-node-back-btn');
  const modbusNodeSaveBtn = document.getElementById('modbus-node-save-btn');
  const modbusNodeResetBtn = document.getElementById('modbus-node-reset-btn');
  const modbusNodeTableWrapper = document.getElementById('modbus-node-table-wrapper');
  const modbusFormSlaveId = document.getElementById('modbus-form-slaveid');
  const modbusFormAddrMode = document.getElementById('modbus-form-addrmode');
  const modbusFormFuntion = document.getElementById('modbus-form-funtion');
  const modbusFormAddress = document.getElementById('modbus-form-address');
  const modbusFormQuantity = document.getElementById('modbus-form-quantity');
  if (modbusNodeAddBtn && modbusNodeForm && modbusNodeBackBtn && modbusNodeTableWrapper) {
      modbusNodeAddBtn.addEventListener('click', () => {
          modbusNodeForm.style.display = '';
          modbusNodeTableWrapper.style.display = 'none';
          modbusNodeAddBtn.style.display = 'none';
          modbusFormSlaveId.value = '';
          modbusFormAddrMode.value = '';
          modbusFormFuntion.value = '';
          modbusFormAddress.value = '';
          modbusFormQuantity.value = '';
      });
      modbusNodeBackBtn.addEventListener('click', () => {
          modbusNodeForm.style.display = 'none';
          modbusNodeTableWrapper.style.display = '';
          modbusNodeAddBtn.style.display = '';
      });
      modbusNodeSaveBtn.addEventListener('click', () => {
          alert(`Saved!\nSlaveID: ${modbusFormSlaveId.value}\nAddress Mode: ${modbusFormAddrMode.value}\nFuntion: ${modbusFormFuntion.value}\nAddress: ${modbusFormAddress.value}\nQuantity: ${modbusFormQuantity.value}`);
          modbusNodeForm.style.display = 'none';
          modbusNodeTableWrapper.style.display = '';
          modbusNodeAddBtn.style.display = '';
      });
      modbusNodeResetBtn.addEventListener('click', () => {
          modbusFormSlaveId.value = '';
          modbusFormAddrMode.value = '';
          modbusFormFuntion.value = '';
          modbusFormAddress.value = '';
          modbusFormQuantity.value = '';
      });
  }

  // Serial Port Settings tab logic
  const serialTabBtns = document.querySelectorAll('.serial-tab-btn');
  const serialBasicForm = document.getElementById('serial-basic-form');
  const serialAdvancedForm = document.getElementById('serial-advanced-form');
  serialTabBtns.forEach(btn => {
      btn.addEventListener('click', () => {
          serialTabBtns.forEach(b => b.classList.remove('active'));
          btn.classList.add('active');
          if (btn.dataset.tab === 'basic') {
              serialBasicForm.style.display = '';
              serialAdvancedForm.style.display = 'none';
          } else {
              serialBasicForm.style.display = 'none';
              serialAdvancedForm.style.display = '';
          }
      });
  });
  // Save/Reset logic for Serial Port Settings
  const serialBasicSaveBtn = document.getElementById('serial-basic-save-btn');
  const serialBasicResetBtn = document.getElementById('serial-basic-reset-btn');
  const serialBasicBuffer = document.getElementById('serial-basic-buffer');
  const serialBasicGap = document.getElementById('serial-basic-gap');
  const serialBasicFlow = document.getElementById('serial-basic-flow');
  const serialBasicCli = document.getElementById('serial-basic-cli');
  const serialBasicProto = document.getElementById('serial-basic-proto');
  if (serialBasicSaveBtn && serialBasicResetBtn) {
      serialBasicSaveBtn.addEventListener('click', () => {
          alert(`Saved!\nBuffer Size: ${serialBasicBuffer.value}\nGap Time: ${serialBasicGap.value}\nFlow Control: ${serialBasicFlow.value}\nCli: ${serialBasicCli.value}\nProtocol: ${serialBasicProto.value}`);
      });
      serialBasicResetBtn.addEventListener('click', () => {
          serialBasicBuffer.value = '512';
          serialBasicGap.value = '50';
          serialBasicFlow.value = 'Disable';
          serialBasicCli.value = 'Disable';
          serialBasicProto.value = 'Modbus';
      });
  }
  const serialAdvSaveBtn = document.getElementById('serial-adv-save-btn');
  const serialAdvResetBtn = document.getElementById('serial-adv-reset-btn');
  const serialAdvBaud = document.getElementById('serial-adv-baud');
  const serialAdvData = document.getElementById('serial-adv-data');
  const serialAdvStop = document.getElementById('serial-adv-stop');
  const serialAdvParity = document.getElementById('serial-adv-parity');
  if (serialAdvSaveBtn && serialAdvResetBtn) {
      serialAdvSaveBtn.addEventListener('click', () => {
          alert(`Saved!\nBaud Rate: ${serialAdvBaud.value}\nData Bit: ${serialAdvData.value}\nStop Bit: ${serialAdvStop.value}\nParity: ${serialAdvParity.value}`);
      });
      serialAdvResetBtn.addEventListener('click', () => {
          serialAdvBaud.value = '9600';
          serialAdvData.value = '8';
          serialAdvStop.value = '1';
          serialAdvParity.value = 'None';
      });
  }

  // Reboot button logic
  const rebootBtn = document.getElementById('reboot-perform-btn');
  if (rebootBtn) {
      rebootBtn.addEventListener('click', () => {
          alert('Rebooting device... (demo)');
      });
  }
  // Backup/Flash firmware button logic
  const backupGenerateBtn = document.getElementById('backup-generate-btn');
  const backupUploadBtn = document.getElementById('backup-upload-btn');
  const backupResetBtn = document.getElementById('backup-reset-btn');
  const backupFlashBtn = document.getElementById('backup-flash-btn');
  if (backupGenerateBtn) backupGenerateBtn.addEventListener('click', () => alert('Generating archive... (demo)'));
  if (backupUploadBtn) backupUploadBtn.addEventListener('click', () => alert('Uploading archive... (demo)'));
  if (backupResetBtn) backupResetBtn.addEventListener('click', () => alert('Factory reset... (demo)'));
  if (backupFlashBtn) backupFlashBtn.addEventListener('click', () => alert('Flashing firmware... (demo)'));
}); 
