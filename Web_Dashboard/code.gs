// ==== 設定區 ====
// 設定你的「基礎溫度」 (Base Temperature)
// 不同作物不同，例如稻米約 10度，糯玉米約 10度，小麥約 0-5度
const BASE_TEMP = 10.0; 
// ================

function doGet(e) {
  // 1. 取得參數 (若沒有參數則給預設值 0，避免報錯)
  var temp = (e.parameter.temp) ? parseFloat(e.parameter.temp) : 0;
  var hum = (e.parameter.hum) ? parseFloat(e.parameter.hum) : 0;
  
  // 2. 取得目前的試算表與 'Data' 分頁
  var ss = SpreadsheetApp.getActiveSpreadsheet();
  var sheet = ss.getSheetByName('Data');
  
  // 3. 取得現在時間
  var date = new Date();
  
  // 4. 寫入資料到最後一行 [時間, 溫度, 濕度]
  sheet.appendRow([date, temp, hum]);
  
  // 5. (選用) 呼叫積溫計算函式 (每次上傳都更新一次積溫表)
  // 如果資料量很大，建議改用「觸發條件」每天定時計算，比較不佔效能
  updateGDD();

  // 6. 回傳成功訊息給 ESP8266
  return ContentService.createTextOutput("Success");
}

// ==== 積溫計算邏輯 (簡易版) ====
function updateGDD() {
  var ss = SpreadsheetApp.getActiveSpreadsheet();
  var dataSheet = ss.getSheetByName('Data');
  
  // 檢查是否有 'Dashboard' 分頁，沒有就建立一個
  var dashSheet = ss.getSheetByName('Dashboard');
  if (!dashSheet) {
    dashSheet = ss.insertSheet('Dashboard');
    dashSheet.appendRow(['日期', '日平均溫', '有效積溫', '累積積溫']); // 標題
  }
  
  // 這裡為了示範簡單，我們只計算「當日」的積溫並顯示在 Dashboard B2 格子
  // 實際農業應用通常是每天結算一筆
  
  // 取得所有數據
  var data = dataSheet.getDataRange().getValues();
  // 略過標題列，從第1行開始
  if (data.length < 2) return;

  var lastRow = data[data.length - 1]; // 最新一筆資料
  var latestTemp = lastRow[1];         // 最新溫度
  
  // 在 Dashboard 顯示最新狀態
  dashSheet.getRange("E1").setValue("目前即時溫度");
  dashSheet.getRange("E2").setValue(latestTemp);
  
  // 實際上，積溫建議在試算表直接用公式 =SUM(...) 計算比較靈活，不建議寫死在 Script 裡
}