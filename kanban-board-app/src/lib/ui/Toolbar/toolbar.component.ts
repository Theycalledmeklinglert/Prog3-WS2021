import { Component, Input } from '@angular/core';

@Component({
  selector: 'toolbar',     // Zugriffstag
  templateUrl: './toolbar.component.html',
  styleUrls: ['./toolbar.component.scss'],
})
export class ToolbarComponent {
  @Input()
  title = "My Board";
}
