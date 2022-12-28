#pragma once

//
//  DlgSaveEmbeddedLyricsController.h
//  MiniLyricsMac
//
//  Created by Xiao Hongyong on 6/7/13.
//
//

#import <Cocoa/Cocoa.h>

@interface DlgSaveEmbeddedLyricsController : NSWindowController<NSTableViewDelegate, NSTableViewDataSource> {
    class CDlgSaveEmbeddedLyrics *mDialg;
    NSMutableArray *_tableDesc;
    NSMutableArray *_tableName;
    NSMutableArray *_tableCheck;
}

@property (assign) IBOutlet NSTableView *tableView;

- (IBAction)buttonOK:(id)sender;
- (IBAction)buttonCancel:(id)sender;

- (void) doModal:(class CDlgSaveEmbeddedLyrics *)dialog;

- (void)addItem:(const char *)text name:(const char *)name isChecked:(bool)isChecked;

- (const char*)getNameAtIndex:(long)index;

- (bool)getCheckAtIndex:(long)index;

- (int)getItemCount;

@end
